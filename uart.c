#include "sys/cdefs.h"
#include <msp430.h>
#include <msp430f6736.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Circular TX buffer (power of 2 size for speed)
// ---------------------------------------------------------------------------

#define TXBUF_SIZE 256
#define TXBUF_MASK (TXBUF_SIZE - 1)
#define RXBUF_SIZE 256
#define RXBUF_MASK (RXBUF_SIZE - 1)

static volatile uint8_t tx_buf[TXBUF_SIZE];
static volatile uint16_t tx_head = 0; // Next byte to send (ISR consumes)
static volatile uint16_t tx_tail = 0; // Next free slot (app fills)

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// UART Init Functions
// ---------------------------------------------------------------------------

/* 9600 BAUD
  P1.4 – UCA1RXD  (UART RX)
  P1.5 – UCA1TXD  (UART TX)
*/
void uart_init_9600_simple(void) {
  // Set UART pin direction
  // P1.4 = UCA1RXD, P1.5 = UCA1TXD
  P1SEL |= BIT4 | BIT5;
  P1DIR |= BIT5;

  UCA1CTL1 |= UCSWRST; // hold in reset
  UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;

  // 16.777216 MHz / (16 * 109.25) ≈ 9598 baud (~0.02% error)
  UCA1BRW = 109;
  UCA1MCTLW = UCOS16 | UCBRF_4; // UCBRS = 0

  UCA1CTL1 &= ~UCSWRST; // release reset
}

/* 9600 BAUD
  P1.4 – UCA1RXD  (UART RX)
  P1.5 – UCA1TXD  (UART TX)
*/
void uart_init_9600(void) {
  /*==========================================================================
    Configure P1.4 and P1.5 as USCI_A1 UART peripheral pins (UCA1RXD / UCA1TXD)
    According to MSP430F6736 pinout and module mapping:
        P1.4 → UCA1RXD (receive)
        P1.5 → UCA1TXD (transmit)
  ==========================================================================*/
  P1SEL |=
      BIT4 | BIT5; // P1SEL.x = 1 → select primary peripheral function (USCI_A1)
                   // This maps P1.4/P1.5 to UCA1RXD/UCA1TXD instead of GPIO

  P1DIR |=
      BIT5; // Set P1.5 (TXD) as output
            // P1.4 (RXD) remains input automatically when in peripheral mode

  /*==========================================================================
    USCI_A1 reset and clock source selection
  ==========================================================================*/
  UCA1CTL1 |= UCSWRST; // Put USCI state machine into software reset (required
                       // before config) UCSWRST = 1 → module held in reset,
                       // registers can be safely written

  UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;
  // UCSSEL__SMCLK → select SMCLK as clock source for baud generation
  // In TIDM-00879: SMCLK = DCO ≈ 16.777216 MHz (exact FLL tuned value)
  // Keep UCSWRST set until configuration complete

  /*==========================================================================
    Baud rate generation: 9600 baud @ SMCLK = 16.777216 MHz
    Using 16× oversampling mode (UCOS16 = 1) for better noise immunity and
  accuracy

    Formula (low-frequency mode with oversampling):
        Baud rate = SMCLK / (UCBRx + UCBRFx/16)

    Target: 16,777,216 Hz / 9600 ≈ 1747.626666...

    Chosen settings:
        UCBRW  = 109   → integer divider = 109
        UCBRF  = 4     → fractional part = 4/16 = 0.25
        Total divider = 109 + 0.25 = 109.25

    Actual baud rate = 16,777,216 / 109.25 ≈ 9608.9 baud
                       → Error = (9608.9 - 9600)/9600 ≈ +0.09% (excellent, well
  under ±2% UART tolerance)
  ==========================================================================*/
  UCA1BRW = 109; // UCBRx = 109 (16-bit value written to UCA1BR0 + UCA1BR1 via
                 // UCA1BRW)

  UCA1MCTLW = UCOS16 // Enable 16× oversampling mode (improves baud accuracy)
              | UCBRF_4; // First modulation stage: UCBRF = 4 → adds 4/16 to
                         // divisor UCBRSx = 0 → second modulation stage unused
                         // (fine with this error)

  /*==========================================================================
    Finalize initialization – release reset
  ==========================================================================*/
  UCA1CTL1 &= ~UCSWRST; // Clear UCSWRST → USCI_A1 now operational
                        // UART is now ready for transmit/receive at 9600 baud
}

/* 115200 BAUD
  ------------------- Port 1 -------------------
  P1.0 – Memory CS (external RAM/Flash chip select)
  P1.2 – UCA1SOMI (eSPI / USCI_A1 SPI)
  P1.3 – UCA1SIMO
  P1.4 – UCA1RXD  (UART RX)
  P1.5 – UCA1TXD  (UART TX)
  P1.6 – UCA1CLK  (SPI clock)
  P1.7 – TEST_EN (test interface power control)
*/
void uart_init_115200(void) {
  // Set UART pin direction
  // P1.4 = RX, P1.5 = TX
  P1SEL |= BIT4 | BIT5;
  P1DIR |= BIT5;

  UCA1CTL1 |= UCSWRST;                // Hold module in reset
  UCA1CTL1 = UCSSEL__SMCLK | UCSWRST; // SMCLK source

  UCA1BRW = 9;                  // Divider for 115200 baud (SMCLK=16.777216 MHz)
  UCA1MCTLW = UCOS16 | UCBRF_2; // Oversampling, fractional=2/16

  UCA1CTL1 &= ~UCSWRST; // Release reset
}


// ---------------------------------------------------------------------------
// Blocking while loop sending methods
// ---------------------------------------------------------------------------

void uart_putc(char c) {
  while (!(UCA1IFG & UCTXIFG));
  UCA1TXBUF = c;
}

void uart_print(const char *str) {
  while (*str)
    uart_putc(*str++);
}

void send_sample(float sample) {

  static uint32_t sample_int;
  sample_int = *((uint32_t *)&sample);

  while ((UCA1STATW & UCBUSY) > 0)
    ; // header byte to sync byte boundary
  UCA1TXBUF = 0xAA;

  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = 0x55; // header byte to sync byte boundary

  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 24) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 16) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 8) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int) & 0xFF);
}

void send_sample_binary(float sample) {
  static uint32_t sample_int = 0;
  sample_int = *((uint32_t *)&sample);

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = 0xAA;

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = 0x55;

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = (uint8_t)(sample_int >> 24);

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = (uint8_t)(sample_int >> 16);

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = (uint8_t)(sample_int >> 8);

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = (uint8_t)(sample_int);
}

void uart_print_safe(const char *str) {
  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = 0xAA;

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = 0xFF; // Escape code: ASCII follows

  while (*str) {
    while (UCA1STATW & UCBUSY)
      ;
    UCA1TXBUF = *str++;
  }

  while (UCA1STATW & UCBUSY)
    ;
  UCA1TXBUF = '\n';
}

void uart_print_integer(int32_t num) {
  char buffer[12];
  unsigned int i = 0;

  if (num == 0) {
    uart_putc('0');
    return;
  }

  if (num < 0) {
    uart_putc('-');
    num = -num;
  }

  while (num > 0) {
    buffer[i++] = (num % 10) + '0';
    num /= 10;
  }
  while (i > 0)
    uart_putc(buffer[--i]);
}

void uart_println(void) { uart_print("\r\n"); }



//==============================================================================
//  Non-blocking, interrupt-driven string print (renamed version)
//==============================================================================
void uart_print_interrupt(const char *s)
{
    if (!s) return;

    __disable_interrupt();

    while (*s)
    {
        uint16_t next = (tx_head + 1) & TXBUF_MASK;

        // wait while buffer is full
        while (next == tx_tail)
        {
            __enable_interrupt();
            __no_operation();       // let the ISR run
            __disable_interrupt();
        }

        tx_buf[tx_head] = *s++;
        tx_head = next;
    }

    // ---- Kick-start the transmitter if it was idle ----
    if (!(UCA1IE & UCTXIE))               // TX interrupt disabled?
    {
        if (tx_head != tx_tail)           // something in buffer?
        {
            UCA1IE   |= UCTXIE;           // enable TX interrupt
            UCA1TXBUF = tx_buf[tx_tail];  // send first byte
            tx_tail    = (tx_tail + 1) & TXBUF_MASK;
        }
    }

    __enable_interrupt();
}

void uart_print_integer_interrupt(uint32_t val) {
    char numbuf[12];
    char *p = numbuf + sizeof(numbuf);
    *--p = '\0';

    do {
        *--p = '0' + (val % 10);
        val /= 10;
    } while (val);

    uart_print_interrupt(p);
}

void uart_print_signed_integer_interrupt(int32_t val) {
    uint32_t magnitude;
    
    // 1. Check and print sign
    if (val < 0) {
        uart_print_interrupt("-");
        // Use -val to get the positive magnitude (or abs(val))
        magnitude = (uint32_t)(-val); 
    } else {
        magnitude = (uint32_t)val;
    }

    // 2. Use existing logic to print the magnitude
    char numbuf[12]; 
    char *p = numbuf + sizeof(numbuf);
    *--p = '\0';

    do {
        *--p = '0' + (magnitude % 10);
        magnitude /= 10;
    } while (magnitude);

    uart_print_interrupt(p);
}

void uart_println_interrupt(void) { uart_print_interrupt("\r\n"); }

// ---------------------------------------------------------------------------
// Internal: queue raw bytes (used by all public functions)
static inline void uart_queue_bytes(const uint8_t *data, uint16_t len) {
  __disable_interrupt();

  uint16_t space = TXBUF_SIZE - (tx_head - tx_tail);
  if (space < len) {
    // Buffer full → drop oldest or block? Here we just drop new data
    // (or you can __bic_SR_register_on_exit(LPM3_bits) to wake and retry)
    __enable_interrupt();
    return;
  }

  while (len--) {
    tx_buf[tx_tail++ & TXBUF_MASK] = *data++;
  }

  // Start transmission if not already running
  if (!(UCA1IE & UCTXIE)) {
    UCA1IE |= UCTXIE;                           // Enable TX interrupt
    UCA1TXBUF = tx_buf[tx_head++ & TXBUF_MASK]; // Prime first byte
  }
  __enable_interrupt();
}

// ---------------------------------------------------------------------------
// Public: Send one float in your original binary format
void send_sample_interrupt(float sample) {
  uint32_t u32 = *((uint32_t *)&sample);
  uint8_t packet[6] = {0xAA,
                       0x55,
                       (uint8_t)(u32 >> 24),
                       (uint8_t)(u32 >> 16),
                       (uint8_t)(u32 >> 8),
                       (uint8_t)(u32 >> 0)};
  uart_queue_bytes(packet, 6);
}

// ---------------------------------------------------------------------------
// Public: Safe ASCII logging (won't corrupt binary stream)
void uart_log_interrupt(const char *msg) {
  uint8_t header[] = {0xAA, 0xFF}; // 0xAA 0xFF = ASCII follows
  uart_queue_bytes(header, 2);
  uart_queue_bytes((const uint8_t *)msg, strlen(msg));

  static const char nl = '\n';
  uart_queue_bytes((const uint8_t *)&nl, 1);
}

/*=====================================================================
  Pre-computed powers of 10 as float – avoids calling powf() at runtime
=====================================================================*/
static const float pow10f[] = {
    1.0f,
    10.0f,
    100.0f,
    1000.0f,
    10000.0f,
    100000.0f,
    1000000.0f,
    10000000.0f
};


/*=====================================================================
  float_to_str() – core conversion routine
  You call this directly if you want the string without printing
=====================================================================*/
void float_to_str(float value, char *buf, uint8_t decimal_places)
{
    char *ptr = buf;
    uint8_t digits;

    // Handle negative numbers
    if (value < 0.0f)
    {
        *ptr++ = '-';
        value = -value;
    }

    // Round correctly to the specified number of decimal places
    float rounding = 0.5f;
    uint8_t i;
    for (i = 0; i < decimal_places; ++i) {
        rounding /= 10.0f; }
    value += rounding;

    // Extract integer part
    uint32_t int_part = (uint32_t)value;
    float remainder = value - (float)int_part;

    // Convert integer part to string (fast integer-only method)
    if (int_part == 0 && value > 0.0f)
    {
        *ptr++ = '0';           // leading zero before decimal point
    }
    else
    {
        char int_buf[11];
        char *p = int_buf + sizeof(int_buf);
        *--p = '\0';
        do {
            *--p = '0' + (int_part % 10);
            int_part /= 10;
        } while (int_part);
        while (*p)
            *ptr++ = *p++;
    }

    // Add decimal point if we need decimals
    if (decimal_places > 0)
    {
        *ptr++ = '.';

        // Extract decimal part
        uint32_t dec_part = (uint32_t)(remainder * pow10f[decimal_places]);
        digits = decimal_places;

        char dec_buf[11];
        char *p = dec_buf + sizeof(dec_buf);
        *--p = '\0';
        do {
            *--p = '0' + (dec_part % 10);
            dec_part /= 10;
        } while (--digits);

        while (*p)
            *ptr++ = *p++;
    }

    *ptr = '\0';  // null terminator
}

/*=====================================================================
  uart_print_float()
  Prints a float with fixed decimal places (e.g. 1.2345 with 4 decimals)
  Handles negative numbers and rounding correctly.
  Max 10 characters total including sign, dot and terminator.
=====================================================================*/
void uart_print_float(float value, uint8_t decimal_places)
{
    char buf[12];
    float_to_str(value, buf, decimal_places);
    uart_print_interrupt(buf);
}


//******************************************************************************
//
// USCI_A1 interrupt vector service routine.
//
//******************************************************************************
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {

  static volatile uint8_t rx_buf[RXBUF_SIZE];
  static volatile uint16_t rx_head = 0; // Next free slot (ISR fills)
  static volatile uint16_t rx_tail = 0; // Next byte to read (app

  switch (__even_in_range(UCA1IV, 4)) {

    // Vector 2 - UCRXIFG — Receive interrupt
  case 2: {
    uint8_t c = UCA1RXBUF; // Read clears the flag
    uint16_t next = (rx_head + 1) & RXBUF_MASK;

    if (next != rx_tail) { // Only store if buffer not full
      rx_buf[rx_head] = c;
      rx_head = next;
    }
    // else: overflow → byte dropped (or set a flag)
    break;
  }

  // ------------------------------------------------------------------
  // Vector 4 – Transmit buffer empty
  // ------------------------------------------------------------------
  case 4: // UCTXIFG
  {
    if (tx_head != tx_tail) // More bytes to send?
    {
      UCA1TXBUF = tx_buf[tx_tail];
      tx_tail = (tx_tail + 1) & TXBUF_MASK;
    } else {
      UCA1IE &= ~UCTXIE; // Nothing left → disable TX IRQ
    }
    break;
  }

  default:
    break;
  }
}
