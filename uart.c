#include <msp430f6736.h>
#include <stdint.h>

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

void uart_init_115200(void)
{
    P1SEL |= BIT4 | BIT5;     // P1.4 = RX, P1.5 = TX
    P1DIR |= BIT5;

    UCA1CTL1 |= UCSWRST;      // Hold module in reset
    UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;   // SMCLK source

    UCA1BRW   = 9;                  // Divider for 115200 baud (SMCLK=16.777216 MHz)
    UCA1MCTLW = UCOS16 | UCBRF_2;   // Oversampling, fractional=2/16

    UCA1CTL1 &= ~UCSWRST;           // Release reset
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

void uart_print_int(int32_t num) {
  char buffer[12];
  int i = 0;

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
