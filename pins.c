// pins.c
#include "dmm_00879_defines.h"
#include "lcd.h"
#include <msp430f6736.h>

/*==============================================================================
  GPIO configuration for general I/O and peripheral mapping
==============================================================================*/
void setup_gpio(void) {
  /*------------------- Port 1 -------------------
    P1.0 – Memory CS (external RAM/Flash chip select)
    P1.2 – UCA1SOMI (eSPI / USCI_A1 SPI)
    P1.3 – UCA1SIMO
    P1.4 – UCA1RXD  (UART RX)
    P1.5 – UCA1TXD  (UART TX)
    P1.6 – UCA1CLK  (SPI clock)
    P1.7 – TEST_EN (test interface power control)
  */
  P1OUT = P1OUT_INIT;     // Initial output latch values (e.g. CS high)
  P1DIR = P1DIR_INIT;     // Direction: 1 = output (BIT0,1,3,6,7)
  P1SEL = P1SEL_INIT;     // Peripheral select (enable USCI_A1 on P1.2–P1.6)
  P1REN = P1REN_INIT;     // Pull-up/down resistors (none used here)
  P1MAP01 = P1MAP01_INIT; // Port mapping for flexible peripheral assignment
                          // (not used)

  /*------------------- Port 2 -------------------
    P2.0 – SCL (EEPROM I²C)
    P2.1 – SDA (EEPROM I²C)
    P2.2–P2.5 – Second SPI for external AFE/test interface
    P2.6 – KEY1 (Mode/Range button)
    P2.7 – KEY2 (Function button)
  */
  P2OUT = P2OUT_INIT; // Pull keys high, enable front-end power (BIT0)
  P2DIR = P2DIR_INIT; // I²C & SPI pins output, keys input
  P2SEL = P2SEL_INIT; // No peripheral function on P2 (all GPIO or I²C via UCB)
  P2REN = P2REN_INIT; // Enable pull-ups on KEY1/KEY2 (BIT6|BIT7)
  P2IES = P2IES_INIT; // Interrupt edge select: high-to-low (button press)
}

/*==============================================================================
  LCD_C module (integrated LCD controller) and segment/com pin configuration
==============================================================================*/
void setup_lcd(void) {
  /* Port 3 – lower nibble used for operating-voltage selection (not used in
   * final design) */
  P3OUT = (BIT2 + BIT1); // Safe default state
  P3DIR = P3DIR_INIT;    // First 4 pins output, upper 4 are LCD segments
  P3SEL = P3SEL_INIT;    // No peripheral function
  P3REN = P3REN_INIT;

  /* Ports 4,5,7 – dedicated LCD segment lines (S0–S39) */
  P4OUT = P4OUT_INIT;
  P4DIR = P4DIR_INIT;
  P4SEL = P4SEL_INIT;
  P4REN = P4REN_INIT;
  P5OUT = P5OUT_INIT;
  P5DIR = P5DIR_INIT;
  P5SEL = P5SEL_INIT;
  P5REN = P5REN_INIT;
  P7OUT = P7OUT_INIT;
  P7DIR = P7DIR_INIT;
  P7SEL = P7SEL_INIT;
  P7REN = P7REN_INIT;

  /* Port 8 – mixed: segments + LEDs + RF test pins */
  P8OUT = P8OUT_INIT;
  P8DIR = P8DIR_INIT;
  P8SEL = P8SEL_INIT;
  P8REN = P8REN_INIT;

  /* Port 9 & Port J – additional segment lines + JTAG (left as GPIO) */
  P9OUT = P9OUT_INIT;
  P9DIR = P9DIR_INIT;
  P9SEL = P9SEL_INIT;
  P9REN = P9REN_INIT;
  PJOUT = PJOUT_INIT;
  PJDIR = PJDIR_INIT;
  PJSEL = PJSEL_INIT;

  /*---------------- LCD_C core settings ----------------*/
  LCDCCTL0 = LCDDIV_2 | LCDPRE_3 | LCD4MUX | LCDON | LCDSSEL;
  // Clock: fLCD = ACLK/(2+1)/(3+1) = ACLK/12 ≈ 2.73 kHz (typical for
  // static/4-mux) 4-mux mode, LCD module on, source select = dedicated LCD pins

  // Charge pump enabled, VLCD = 2.84 V (typical for good contrast)
  LCDCVCTL = LCDCPEN | VLCD_2_84; // LCDCPEN – internal charge pump enable
  REFCTL0 &= ~REFMSTR; // Ensure REF module does not override LCD reference

  // Enable COM/SEG pins (all segments used except a few reserved ones)
  LCDCPCTL0 = 0xFFFF; // LCDS0 – LCDS15
  LCDCPCTL1 = 0xFFFF; // LCDS16–LCDS31
  LCDCPCTL2 = 0xFFC3; // LCDS32–LCDS39 + workaround bits (TI errata)

  LCDCMEMCTL = LCDCLRM; // Clear all LCD memory (all segments off)
  LCDCBLKCTL = LCDBLKDIV__6 | LCDBLKPRE_3 | LCDBLKMOD_0;
  // Blinking clock ≈ 1 Hz (used for overflow indication)
}

/*==============================================================================
  SD24_B (24-bit Sigma-Delta ADC) configuration – dual channel (V & I)
==============================================================================*/
void setup_adc(void) {
  // Ensure converter 0 start does not trigger group start for converter 1
  SD24BCTL1 &= ~SD24GRP0SC; // Disable group start chaining

  // Global SD24_B settings
  SD24BCTL0 = SD24SSEL__SMCLK | SD24REFS;
  // SD24SSEL__SMCLK → modulator clock = SMCLK (2MHz)
  // SD24REFS        → internal 1.2 V reference selected

  /*--------------- Channel 0 – Voltage measurement ---------------*/
  SD24BINCTL0 =
      (SD24B_VOLTAGE_PGA_GAIN << 3); // Same gain options as current channel
  SD24BCCTL0 = SD24DF_1;             // 2’s complement, right-aligned
  SD24BOSR0 = SD24B_OSR_RATIO - 1;   // Same OSR as current channel
  SD24BPRE0 = 0;                     // No additional pre-divider

  /*--------------- Channel 1 – Current measurement ---------------*/
  SD24BINCTL1 = (SD24B_CURRENT_PGA_GAIN
                 << 3);  // Gain = 1, 2, 4, 8, 16, 32, 64 (here 16× typical)
  SD24BCCTL1 = SD24DF_1; // Data format: 2’s complement, right-aligned
  SD24BOSR1 =
      SD24B_OSR_RATIO - 1; // Oversampling ratio (64 typical → decimation = 64)
  SD24BPRE1 = 0;           // No additional pre-divider

  /*--------------- Channel 2 – OHM measurement ---------------*/
  SD24BINCTL2 = (SD24B_CURRENT_PGA_GAIN
                 << 3);  // Gain = 1, 2, 4, 8, 16, 32, 64 (here 16× typical)
  SD24BCCTL2 = SD24DF_1; // Data format: 2’s complement, right-aligned
  SD24BOSR2 =
      SD24B_OSR_RATIO - 1; // Oversampling ratio (64 typical → decimation = 64)
  SD24BPRE2 = 0;           // No additional pre-divider

  // Enable group start (SD24SC bit for channel 0 triggers all three converters)
  SD24BCTL1 |= SD24GRP0SC;

  // Enable interrupt for voltage channel (current channel shares same vector)
  SD24BIE |= SD24IE0; // SD24IE0 → interrupt enable for converter 0
}
