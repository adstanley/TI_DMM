#define __MAIN_PROGRAM__
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>
#include "dmm_00879_hw_setup.h"

void test_interface_power_down() {
  P1SEL = 0;
  P1OUT = 0;
  P1DIR = 0xFF;
}

void uart_init(void)
{
    P1SEL |= BIT4 | BIT5;      // UCA1RXD / UCA1TXD
    P1DIR |= BIT5;

    UCA1CTL1 |= UCSWRST;
    UCA1CTL1 |= UCSSEL_2;      // SMCLK
    UCA1BRW   = 13;
    UCA1MCTLW = 0xB6A1;        // 9600 baud @ ~16 MHz exact
    UCA1CTL1 &= ~UCSWRST;
}

// Send a single character via UCA1
void uart_putc(char c) {
    // Poll the Interrupt Flag Register: Wait until the Transmit Buffer is empty
    // UCTXIFG is defined in msp430f6736.h
    while (!(UCA1IFG & UCTXIFG));

    // Write the character to the Transmit Buffer
    UCA1TXBUF = c;
}

// Send a null-terminated string
void uart_print(const char *str) {
    while (*str) {
        uart_putc(*str);
        str++;
    }
}

// Send an integer (useful for printing your measured values)
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

    // Extract digits in reverse order
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    // Print digits in correct order
    while (i > 0) {
        uart_putc(buffer[--i]);
    }
}

// Print a New Line (Carriage Return + Line Feed)
void uart_println(void) {
    uart_print("\r\n");
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    system_setup();            // Keep the original clock setup (or at least the
lines above) uart_init();               // Or rely on system_setup() which
already does it

    uart_print("\r\nTI DMM-00879 UART Test - Working!\r\n");

    int counter = 0;
    while (1)
    {
        uart_print("Heartbeat: ");
        uart_print_int(counter++);
        uart_print("\r\n");
        __delay_cycles(16000000);   // ~1 second at 16 MHz
    }
}
