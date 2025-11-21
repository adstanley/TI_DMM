#include <msp430.h>
#include <msp430f6736.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Circular TX buffer (power of 2 size for speed)
// ---------------------------------------------------------------------------

#define TXBUF_SIZE 256
#define TXBUF_MASK (TXBUF_SIZE - 1)

static volatile uint8_t tx_buf[TXBUF_SIZE];
static volatile uint16_t tx_head = 0;      // Next byte to send (ISR consumes)
static volatile uint16_t tx_tail = 0;      // Next free slot (app fills)

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// UART Init Functions
// ---------------------------------------------------------------------------

// 9600 BAUD
void uart_init_9600(void) {
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

// 115200 BAUD
void uart_init_115200(void) {
  P1SEL |= BIT4 | BIT5; // P1.4 = RX, P1.5 = TX
  P1DIR |= BIT5;

  UCA1CTL1 |= UCSWRST;                // Hold module in reset
  UCA1CTL1 = UCSSEL__SMCLK | UCSWRST; // SMCLK source

  UCA1BRW = 9;                  // Divider for 115200 baud (SMCLK=16.777216 MHz)
  UCA1MCTLW = UCOS16 | UCBRF_2; // Oversampling, fractional=2/16

  UCA1CTL1 &= ~UCSWRST; // Release reset
}

// ---------------------------------------------------------------------------

void send_sample(float sample) {

  static uint32_t sample_int;
  sample_int = *((uint32_t *)&sample);

  while ((UCA1STATW & UCBUSY) > 0); // header byte to sync byte boundary
  UCA1TXBUF = 0xAA;

  while ((UCA1STATW & UCBUSY) > 0);
  UCA1TXBUF = 0x55; // header byte to sync byte boundary

  while ((UCA1STATW & UCBUSY) > 0);
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 24) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0);
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 16) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0);
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 8) & 0xFF);

  while ((UCA1STATW & UCBUSY) > 0);
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int) & 0xFF);
}

// Send binary float (your existing format)
void send_sample_binary(float sample) {
    static uint32_t sample_int = 0;
    sample_int = *((uint32_t *)&sample);

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = 0xAA;

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = 0x55;

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = (uint8_t)(sample_int >> 24);

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = (uint8_t)(sample_int >> 16);

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = (uint8_t)(sample_int >> 8);

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = (uint8_t)(sample_int);
}

void uart_putc(char c) {
  while (!(UCA1IFG & UCTXIFG))
    ;
  UCA1TXBUF = c;
}

void uart_print(const char *str) {
  while (*str)
    uart_putc(*str++);
}

// New: Send ASCII text safely
void uart_print_safe(const char *str) {
    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = 0xAA;

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = 0xFF;        // Escape code: ASCII follows

    while (*str) {
        while (UCA1STATW & UCBUSY);
        UCA1TXBUF = *str++;
    }

    while (UCA1STATW & UCBUSY);
    UCA1TXBUF = '\n';
}

void uart_print_int(int32_t num) {
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

void uart_println(void) { 
  uart_print("\r\n"); 
  }

// ---------------------------------------------------------------------------
// Internal: queue raw bytes (used by all public functions)
static inline void uart_queue_bytes(const uint8_t *data, uint16_t len)
{
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
        UCA1IE |= UCTXIE;                   // Enable TX interrupt
        UCA1TXBUF = tx_buf[tx_head++ & TXBUF_MASK];  // Prime first byte
    }
    __enable_interrupt();
}

// ---------------------------------------------------------------------------
// Public: Send one float in your original binary format
void send_sample_interrupt(float sample)
{
    uint32_t u32 = *((uint32_t*)&sample);
    uint8_t packet[6] = {
        0xAA,
        0x55,
        (uint8_t)(u32 >> 24),
        (uint8_t)(u32 >> 16),
        (uint8_t)(u32 >>  8),
        (uint8_t)(u32 >>  0)
    };
    uart_queue_bytes(packet, 6);
}

// ---------------------------------------------------------------------------
// Public: Safe ASCII logging (won't corrupt binary stream)
void uart_log_interrupt(const char *msg)
{
    uint8_t header[] = { 0xAA, 0xFF };          // 0xAA 0xFF = ASCII follows
    uart_queue_bytes(header, 2);
    uart_queue_bytes((const uint8_t*)msg, strlen(msg));

    static const char nl = '\n';
    uart_queue_bytes((const uint8_t*)&nl, 1);
}

// ---------------------------------------------------------------------------
// USCI_A1 Interrupt (TX only) – TI compiler style
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch (UCA1IV) {
        case 4:                                        // UCTXIFG
            if (tx_head != tx_tail) {
                UCA1TXBUF = tx_buf[tx_head++ & TXBUF_MASK];
            } else {
                UCA1IE &= ~UCTXIE;                     // Buffer empty → disable IRQ
            }
            break;
        default:
            break;
    }
}
