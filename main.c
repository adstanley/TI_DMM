#include "in430.h"
#define __MAIN_PROGRAM__
#include <msp430f6736.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include "clock.h"
#include "dmm_00879_hw_setup.h"
#include "hal_pmm.h"
#include "timer.h"
#include "uart.h"

void uart_print_system_state(void);
static void uart_printf(const char *fmt, ...);

// -----------------------------------------------------------------
// MAIN – External 32.768 kHz Crystal + 16.78 MHz MCLK/SMCLK
// -----------------------------------------------------------------
int main(void) {
  __low_level_init();

  Clock_Init_16MHz();

  // === UART Init ===
  uart_init_9600();
  // uart_init_115200();

  // Enable Interrupts
  _EINT();

  // === Boot message ===
  uart_printf("\r\n\r\n");
  uart_printf("****************************************\r\n");
  uart_printf("* TI DMM-00879 – External Crystal Test *\r\n");
  uart_printf("* 32.768 kHz XT1 → 16.78 MHz MCLK/SMCLK*\r\n");
  uart_printf("* UART 112500 baud                     *\r\n");
  uart_printf("****************************************\r\n\r\n");

  uart_println_interrupt();
  uart_print_system_state();

  // uint32_t counter = 0;

  // while (1) {
  //   uart_print_interrupt("Heartbeat: ");
  //   uart_print_integer_interrupt(counter++);
  //   uart_println_interrupt();
  //   delay_ms(1000);
  //   //__delay_cycles(16777216); // ~1 second at 16.777216 MHz
  // }
}

// Helper to print unsigned numbers in any base (10 or 16)
void uart_print_u32(uint32_t n, int base, int width, char pad) {
  char buf[11]; // Buffer for 32-bit int (max 10 digits + null)
  int i = 0;

  if (n == 0) {
    if (width > 0) {
      while (--width > 0)
        uart_putc(pad);
    }
    uart_putc('0');
    return;
  }

  while (n > 0) {
    int d = n % base;
    buf[i++] = (d < 10) ? (d + '0') : (d - 10 + 'A');
    n /= base;
  }

  if (width > i) {
    width -= i;
    while (width-- > 0)
      uart_putc(pad);
  }

  while (i > 0) {
    uart_putc(buf[--i]);
  }
}

// Helper to print signed 32-bit numbers
void uart_print_i32(int32_t n) {
  if (n < 0) {
    uart_putc('-');
    n = -n;
  }
  uart_print_u32((uint32_t)n, 10, 0, ' ');
}

// Lightweight printf implementation
// Supports: %c, %s, %d (16-bit), %ld (32-bit), %x (16-bit hex), %lx (32-bit
// hex) Added basic width/padding support for %04X, %02X
void uart_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (*format == '%') {
      format++; // Move past '%'

      // Handle zero padding (e.g. %04X)
      char pad = ' ';
      int width = 0;
      if (*format == '0') {
        pad = '0';
        format++;
      }
      while (*format >= '0' && *format <= '9') {
        width = width * 10 + (*format - '0');
        format++;
      }

      switch (*format) {
      case 'c':
        uart_putc((char)va_arg(args, int));
        break;
      case 's':
        uart_print(va_arg(args, char *));
        break;
      case 'd':
        uart_print_i32(va_arg(args, int));
        break;
      case 'X': // fallthrough
      case 'x':
        uart_print_u32(va_arg(args, unsigned int), 16, width, pad);
        break;
      case 'l':   // Handle long (32-bit) modifiers
        format++; // Move past 'l'
        if (*format == 'd') {
          uart_print_i32(va_arg(args, long));
        } else if (*format == 'x' || *format == 'X') {
          uart_print_u32(va_arg(args, unsigned long), 16, width, pad);
        }
        break;
      case '%':
        uart_putc('%');
        break;
      default:
        uart_putc('%');
        uart_putc(*format);
        break;
      }
    } else {
      uart_putc(*format);
    }
    format++;
  }

  va_end(args);
}

// --- SYSTEM STATE DUMP FUNCTION ---
void uart_print_system_state(void) {
  uart_printf("\r\n=== TIDM-00879 DMM System State Dump ===\r\n");
  uart_printf("Compiled: " __DATE__ " " __TIME__ "\r\n\r\n");

  // ----------------------------------------------------------------
  // Clock system (UCS + PMM)
  // ----------------------------------------------------------------
  uart_printf("Clock System\r\n");
  uart_printf("UCSCTL0  = 0x%04X\r\n", UCSCTL0);
  uart_printf("UCSCTL1  = 0x%04X  (DCORSEL=%d)\r\n", UCSCTL1,
              (UCSCTL1 >> 4) & 0x7);
  uart_printf("UCSCTL2  = 0x%04X  (FLLN=%d, FLLD=%d)\r\n", UCSCTL2,
              UCSCTL2 & 0x3FF, (UCSCTL2 >> 11) & 0x3);
  uart_printf("UCSCTL3  = 0x%04X  (SELREF=%d)\r\n", UCSCTL3, UCSCTL3 & 0x7);
  uart_printf("UCSCTL4  = 0x%04X  (SELA=%d, SELS=%d, SELM=%d)\r\n", UCSCTL4,
              UCSCTL4 & 0x7, (UCSCTL4 >> 4) & 0x7, (UCSCTL4 >> 8) & 0x7);
  uart_printf("UCSCTL5  = 0x%04X  (DIVA=%d, DIVS=%d, DIVM=%d)\r\n", UCSCTL5,
              UCSCTL5 & 0x7, (UCSCTL5 >> 4) & 0x7, (UCSCTL5 >> 8) & 0x7);
  uart_printf("UCSCTL6  = 0x%04X\r\n", UCSCTL6);
  uart_printf("UCSCTL7  = 0x%04X\r\n", UCSCTL7);
  uart_printf("UCSCTL8  = 0x%04X\r\n\r\n", UCSCTL8);

  // ----------------------------------------------------------------
  // Power Management (PMM)
  // ----------------------------------------------------------------
  uart_printf("--- Power Management ---\r\n");
  uart_printf("PMMCTL0  = 0x%04X\r\n", PMMCTL0);
  uart_printf("PMMCTL1  = 0x%04X\r\n", PMMCTL1);
  uart_printf("SVSMHCTL = 0x%04X\r\n", SVSMHCTL);
  uart_printf("SVSMLCTL = 0x%04X\r\n\r\n", SVSMLCTL);

  // ----------------------------------------------------------------
  // LCD_C Module
  // ----------------------------------------------------------------
  uart_printf("--- LCD Setup ---\r\n");
  uart_printf("LCDCCTL0 = 0x%04X  (LCDON=%d, LCD4MUX=%d, LCDSSEL=%d)\r\n",
              LCDCCTL0, (LCDCCTL0 & LCDON) ? 1 : 0,
              (LCDCCTL0 & LCD4MUX) ? 1 : 0, (LCDCCTL0 & LCDSSEL) ? 1 : 0);
  uart_printf("LCDCVCTL = 0x%04X  (VLCD=%d, LCDCPEN=%d)\r\n", LCDCVCTL,
              LCDCVCTL & 0x1F, (LCDCVCTL & LCDCPEN) ? 1 : 0);
  uart_printf("LCDCPCTL0 = 0x%04X\r\n", LCDCPCTL0);
  uart_printf("LCDCPCTL1 = 0x%04X\r\n", LCDCPCTL1);
  uart_printf("LCDCPCTL2 = 0x%04X\r\n", LCDCPCTL2);
  uart_printf("LCDCMEMCTL = 0x%04X\r\n", LCDCMEMCTL);
  uart_printf("LCDCBLKCTL = 0x%04X\r\n\r\n", LCDCBLKCTL);

  // ----------------------------------------------------------------
  // SD24_B (24-bit Sigma-Delta ADC) – the heart of the DMM
  // ----------------------------------------------------------------
  uart_printf("--- ADC System ---");
  uart_printf("SD24BCTL0 = 0x%04X  (REFS=%d, SSEL=%d)\r\n", SD24BCTL0,
              (SD24BCTL0 & SD24REFS) ? 1 : 0, SD24BCTL0 & 0x03);
  uart_printf("SD24BCTL1 = 0x%04X  (GRP0SC=%d)\r\n\n", SD24BCTL1,
              (SD24BCTL1 & SD24GRP0SC) ? 1 : 0);

  // Channel 0 – Voltage
  uart_printf("--- Voltage Channel (Ch0) ---\r\n");
  uart_printf("SD24BINCTL0 = 0x%02X  (GAIN=%dx)\r\n", SD24BINCTL0,
              1 << ((SD24BINCTL0 >> 3) & 0x7));
  uart_printf("SD24BCCTL0  = 0x%04X\r\n", SD24BCCTL0);
  uart_printf("SD24BOSR0   = %d\r\n", SD24BOSR0 + 1);
  uart_printf("SD24BIE     bit0 = %d\r\n\n", (SD24BIE & SD24IE0) ? 1 : 0);

  // Channel 1 – Current
  uart_printf("--- Current Channel (Ch1) ---\r\n");
  uart_printf("SD24BINCTL1 = 0x%02X  (GAIN=%dx)\r\n", SD24BINCTL1,
              1 << ((SD24BINCTL1 >> 3) & 0x7));
  uart_printf("SD24BCCTL1  = 0x%04X\r\n", SD24BCCTL1);
  uart_printf("SD24BOSR1   = %d\r\n\n", SD24BOSR1 + 1);

  // Channel 2 - Ohm
    uart_printf("--- Current Channel (Ch2) ---\r\n");
  uart_printf("SD24BINCTL2 = 0x%02X  (GAIN=%dx)\r\n", SD24BINCTL2,
              1 << ((SD24BINCTL1 >> 3) & 0x7));
  uart_printf("SD24BCCTL2  = 0x%04X\r\n", SD24BCCTL2);
  uart_printf("SD24BOSR2   = %d\r\n\n", SD24BOSR2 + 1);

  // ----------------------------------------------------------------
  // UART (USCI_A1) – confirm baud settings
  // ----------------------------------------------------------------
  uart_printf("--- UART --- ");
  uart_printf("UCA1CTL0  = 0x%02X\r\n", UCA1CTL0);
  uart_printf("UCA1CTL1  = 0x%02X  (SWRST=%d, SEL=%d)\r\n", UCA1CTL1,
              (UCA1CTL1 & UCSWRST) ? 1 : 0, (UCA1CTL1 >> 6) & 0x3);
  uart_printf("UCA1BRW   = %d\r\n", UCA1BRW);
  uart_printf("UCA1MCTLW = 0x%04X  (OS16=%d, BRF=%d)\r\n", UCA1MCTLW,
              (UCA1MCTLW & UCOS16) ? 1 : 0, (UCA1MCTLW >> 4) & 0xF);
  uart_printf("UCA1STATW = 0x%02X\r\n", UCA1STATW);
  uart_printf("UCA1IE    = 0x%02X\r\n", UCA1IE);
  uart_printf("UCA1IFG   = 0x%02X\r\n\n", UCA1IFG);

  // ----------------------------------------------------------------
  // Key GPIO & front-end control
  // ----------------------------------------------------------------
  uart_printf("--- GPIO ---\r\n");
  uart_printf("Front-end power (P2.0) = %d\r\n", (P2OUT & BIT0) ? 1 : 0);
  uart_printf("KEY1 (P2.6) pullup = %d, state = %d\r\n", (P2REN & BIT6) ? 1 : 0,
              (P2IN & BIT6) ? 1 : 0);
  uart_printf("KEY2 (P2.7) pullup = %d, state = %d\r\n\n",
              (P2REN & BIT7) ? 1 : 0, (P2IN & BIT7) ? 1 : 0);

  uart_printf("=== End of System State ===\r\n\r\n");
}
