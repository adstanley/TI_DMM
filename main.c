#define __MAIN_PROGRAM__
#include "clock.h"
#include "dmm_00879_hw_setup.h" // still needed for SetVCore() and hal_PMM.h
#include "hal_pmm.h"
#include "timer.h"
#include "uart.h"
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>

int __low_level_init(void) // low level init function
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  return 1;
}

// -----------------------------------------------------------------
// MAIN – External 32.768 kHz Crystal + 16.78 MHz MCLK/SMCLK
// -----------------------------------------------------------------
int main(void) {
  __low_level_init();

  Clock_Init_16MHz_XT1();

  // SetVCore(PMMCOREV_2); // Required for 16+ MHz

  // === UART Init ===
  uart_init_115200();

  // === Boot message ===
  uart_print("\r\n\r\n");
  uart_print("****************************************\r\n");
  uart_print("* TI DMM-00879 – External Crystal Test *\r\n");
  uart_print("* 32.768 kHz XT1 → 16.78 MHz MCLK/SMCLK*\r\n");
  uart_print("* UART 112500 baud                     *\r\n");
  uart_print("****************************************\r\n\r\n");

  __enable_interrupt();

  uint32_t counter = 0;

  while (1) {
    uart_print("Heartbeat: ");
    uart_print_int(counter++);
    uart_println();
    delay_ms(1000);
    //__delay_cycles(16777216); // ~1 second at 16.777216 MHz
  }
}
