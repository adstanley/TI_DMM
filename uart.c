/*==============================================================================
  UART.c - UART Functions
==============================================================================*/

#include <msp430f6736.h>
#include "dmm_00879_processing.h"
#include "uart.h"
#include "dmm_00879_main.h"      // for measurement_mode, displayed_result_fp, etc.
#include "dmm_00879_defines.h"   // for mode defines

// ================================================================
// Retarget printf() to UCA1 (9600 baud by default)
// ================================================================
int __putchar(int ch)
{
    while (!(UCA1IFG & UCTXIFG));   // Wait until TX buffer is ready
    UCA1TXBUF = (unsigned char)ch;
    return ch;
}

// Optional dummy getchar (needed for some stdio implementations)
int __getchar(void)
{
    return -1;  // we don't use input
}

// ================================================================
// UART Initialization – call from system_setup()
// ================================================================
void uart_init(void)
{
    // UCA1 is already configured in hw_setup.c for 9600 baud
    // We just make sure it's running (in case someone disabled it)
    UCA1CTL1 &= ~UCSWRST;   // Bring USCI out of reset
}

// ================================================================
// Print one line with the current reading
// ================================================================
void uart_print_measurement(void)
{
    const char *mode_str[] = {
        [DC_VOLTAGE] = "DC V",
        [AC_VOLTAGE] = "AC V",
        [DC_CURRENT] = "DC I",
        [AC_CURRENT] = "AC I",
        [POWER]      = "POWR",
        [OFF]        = "OFF "
    };

    const char *unit = "V";
    if (measurement_mode == DC_CURRENT || measurement_mode == AC_CURRENT)
        unit = "A";
    else if (measurement_mode == POWER)
        unit = "W";

    printf("%s  %8.4f %s%s\r\n",
           mode_str[measurement_mode],
           displayed_result_fp,
           unit,
           (range_overflow > 0) ? "  OL" : "");
}