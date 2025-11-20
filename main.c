#define __MAIN_PROGRAM__
#include "dmm_00879_hw_setup.h" // still needed for SetVCore() and hal_PMM.h
#include "hal_pmm.h"
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>
#include "clock.h"
#include "uart.h"


// -----------------------------------------------------------------
// UART Helper Functions (unchanged – perfect as-is)
// -----------------------------------------------------------------
// void uart_init(void)
// {
//     P1SEL |= BIT4 | BIT5;                   // P1.4 = UCA1RXD, P1.5 = UCA1TXD
//     P1DIR |= BIT5;

//     UCA1CTL1 |= UCSWRST;
//     UCA1CTL1 |= UCSSEL__SMCLK;             // SMCLK = 16.777216 MHz

//     UCA1BRW   = 109;                       // Integer divider
//     UCA1MCTLW = 0xD641;                    // USCI modulation (UCBRF=4,
//     UCBRS=0xD6, UCOS16=1)

//     UCA1CTL1 &= ~UCSWRST;
// }

// void uart_init(void)
// {
//     // P1.4 = UCA1RXD, P1.5 = UCA1TXD
//     P1SEL |= BIT4 | BIT5;
//     P1DIR |= BIT5;

//     UCA1CTL1 |= UCSWRST;             // hold in reset
//     UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;

//     // 16.777216 MHz / (16 * 109.25) ≈ 9598 baud (~0.02% error)
//     UCA1BRW   = 109;
//     UCA1MCTLW = UCOS16 | UCBRF_4;    // UCBRS = 0

//     UCA1CTL1 &= ~UCSWRST;           // release reset
// }


// void uart_putc(char c) {
//   while (!(UCA1IFG & UCTXIFG))
//     ;
//   UCA1TXBUF = c;
// }

// void uart_print(const char *str) {
//   while (*str)
//     uart_putc(*str++);
// }

// void uart_print_int(int32_t num) {
//   char buffer[12];
//   int i = 0;

//   if (num == 0) {
//     uart_putc('0');
//     return;
//   }
//   if (num < 0) {
//     uart_putc('-');
//     num = -num;
//   }

//   while (num > 0) {
//     buffer[i++] = (num % 10) + '0';
//     num /= 10;
//   }
//   while (i > 0)
//     uart_putc(buffer[--i]);
// }

// void uart_println(void) { uart_print("\r\n"); }

// void Clock_Init_16MHz_XT1(void) {

//   // Assume WDT was stopped in main() or startup code.
//   P5SEL |= BIT0 | BIT1;

//   UCSCTL6 |= XT2OFF;
//   UCSCTL6 &= ~XT1OFF;

//   // Use internal ~12.5 pF caps (XCAP_3)
//   UCSCTL6 = (UCSCTL6 & ~(XCAP0 | XCAP1)) | XCAP_3;

//   UCSCTL6 &= ~(XT1BYPASS | XTS);
//   UCSCTL6 = (UCSCTL6 & ~XT1DRIVE_3) | XT1DRIVE_2;

//   do {
//     UCSCTL7 &= ~(DCOFFG | XT1LFOFFG | XT2OFFG);
//     SFRIFG1 &= ~OFIFG;
//   } while (SFRIFG1 & OFIFG);

//   UCSCTL6 = (UCSCTL6 & ~XT1DRIVE_3) | XT1DRIVE_0;

//   // --- 4. Configure FLL: DCO ~16.777216 MHz from XT1 ---
//   __bis_SR_register(SCG0); // Disable FLL control loop

//   UCSCTL0 = 0x0000;    // Lowest DCOx, MODx
//   UCSCTL1 = DCORSEL_5; // DCO range 16–33 MHz

//   // Fdco = F_ref * (N + 1) * (1 / FLLD)
//   // F_ref = 32.768 kHz, choose N = 511, FLLD = 1:
//   // Fdco = 32768 * 512 = 16.777216 MHz
//   UCSCTL2 = FLLD__1 | (511);

//   // FLL reference from XT1, no FLL ref divider
//   // UCSCTL3 = SELREF__XT1CLK | FLLREFDIV__1;
//   UCSCTL3 |= SELREF_2; // FLL ref clock to RFOCLK

//   __bic_SR_register(SCG0); // Re-enable FLL

//   // --- 5. Select clock sources and dividers ---
//   UCSCTL4 = SELA__XT1CLK    // ACLK  = XT1
//             | SELS__DCOCLK  // SMCLK = DCO
//             | SELM__DCOCLK; // MCLK  = DCO

//   UCSCTL5 = DIVA__1    // ACLK divider  = /1
//             | DIVS__1  // SMCLK divider = /1
//             | DIVM__1; // MCLK divider  = /1

//   // --- 6. Give DCO time to settle ---
//   // This delay is approximate; after this, FLL should be locked.
//   __delay_cycles(500000); // ~30 ms at ~16 MHz
// }

// -----------------------------------------------------------------
// MAIN – External 32.768 kHz Crystal + 16.78 MHz MCLK/SMCLK
// -----------------------------------------------------------------
int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog

  Clock_Init_16MHz_XT1();

  SetVCore(PMMCOREV_2); // Required for 16+ MHz

  // === UART Init ===
  uart_init_115200();

  // === Boot message ===
  uart_print("\r\n\r\n");
  uart_print("****************************************\r\n");
  uart_print("* TI DMM-00879 – External Crystal Test *\r\n");
  uart_print("* 32.768 kHz XT1 → 16.78 MHz MCLK/SMCLK*\r\n");
  uart_print("* UART 9600 baud – ROCK SOLID          *\r\n");
  uart_print("****************************************\r\n\r\n");

  __enable_interrupt();

  uint32_t counter = 0;
  while (1) {
    uart_print("Heartbeat: ");
    uart_print_int(counter++);
    uart_println();
    __delay_cycles(16777216); // ~1 second at 16.777216 MHz
  }
}
