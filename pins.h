#ifndef __PINS_H
#define __PINS_H

/*
 * PORT 1 CONFIGURATION
 * P1.0 (Out) - Memory CS
 * P1.1 (Out) - NC (Not Connected)
 * P1.2 (Sel) - UCA0SOMI (SPI Master In)
 * P1.3 (Sel) - UCA0SIMO (SPI Master Out)
 * P1.4 (Sel) - UCA0RXD (UART RX)
 * P1.5 (Sel) - UCA0TXD (UART TX)
 * P1.6 (Sel) - UCA0CLK (SPI Clock)
 * P1.7 (Out) - TEST_EN
 */

#define P1DIR_INIT (BIT0 | BIT1 | BIT3 | BIT6 | BIT7)
#define P1SEL_INIT (BIT2 | BIT3 | BIT4 | BIT5 | BIT6)
#define P1OUT_INIT (BIT0)
#define P1REN_INIT (0)
#define P1MAP01_INIT (0)

/*
 * PORT 2 CONFIGURATION
 * P2.0 (Out) - SCL for EEPROM (Bit-banged or I2C) / Front End Enable
 * P2.1 (Out) - SDA for EEPROM / Voltage Range LSB
 * P2.2 (Out) - A2SOMI / Voltage Range MSB
 * P2.3 (Out) - A2SIMO / Current Range
 * P2.4 (Out) - A2CS / Battery Test Enable
 * P2.5 (Out) - A2CLK
 * P2.6 (In)  - Key1 (Input with Pull-up)
 * P2.7 (In)  - Key2 (Input with Pull-up)
 */

#define P2DIR_INIT (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)
#define P2SEL_INIT (0)
#define P2OUT_INIT (BIT0 | BIT6 | BIT7)
#define P2REN_INIT (BIT6 | BIT7)
#define P2IES_INIT (BIT6 | BIT7)

/*
 * PORT 3 CONFIGURATION
 * P3.0 - P3.3 (Out) - Operating Voltage Select / Mux Control
 * P3.4 - P3.7 (Out) - LCD Segment lines (Controlled by LCD Controller peripheral usually)
 */

/*
    P3.0 = Operating Voltage Select (MSB)
    P3.1 = Operating Voltage Select
    P3.2 = Operating Voltage Select
    P3.3 = Operating Voltage Select
    P3.4 = LCD segment line
    P3.5 = LCD segment line
    P3.6 = LCD segment line
    P3.7 = LCD segment line
*/

#define P3DIR_INIT 0x0F // Lower nibble output
#define P3SEL_INIT 0
#define P3OUT_INIT (BIT0)
#define P3REN_INIT 0

/*
    P4.0 = LCD segment line
    P4.1 = LCD segment line
    P4.2 = LCD segment line
    P4.3 = LCD segment line
    P4.4 = LCD segment line
    P4.5 = LCD segment line
    P4.6 = LCD segment line
    P4.7 = LCD segment line
 */

#define P4DIR_INIT                                  0x00
#define P4SEL_INIT                                  0
#define P4OUT_INIT                                  0
#define P4REN_INIT                                  0

/*
    P5.0 = LCD segment line
    P5.1 = LCD segment line
    P5.2 = LCD segment line
    P5.3 = LCD segment line
    P5.4 = LCD segment line
    P5.5 = LCD segment line
    P5.6 = LCD segment line
    P5.7 = LCD segment line
*/
#define P5DIR_INIT                                  0
#define P5SEL_INIT                                  0
#define P5OUT_INIT                                  0
#define P5REN_INIT                                  0

/*
    P7.0 = LCD segment line
    P7.1 = LCD segment line
    P7.2 = LCD segment line
    P7.3 = LCD segment line
    P7.4 = LCD segment line
    P7.5 = LCD segment line
    P7.6 = LCD segment line
    P7.7 = LCD segment line
 */
#define P7DIR_INIT                                  0
#define P7SEL_INIT                                  0
#define P7OUT_INIT                                  0
#define P7REN_INIT                                  0

/*
    P8.0 = LCD segment line
    P8.1 = LCD segment line
    P8.2 = LCD segment line
    P8.3 = LCD segment line
    P8.4 = RF
    P8.5 = RF
    P8.6 = LED 1
    P8.7 = LED 2
 */
#define P8DIR_INIT                                  0xF0
#define P8SEL_INIT                                  0
#define P8OUT_INIT                                  (0)
#define P8REN_INIT                                  0

/*
    P9.0 =
    P9.1 =
    P9.2 =
    P9.3 =
    P9.4 =
    P9.5 =
    P9.6 =
    P9.7 =
 */
#define P9DIR_INIT                                  0x0F
#define P9SEL_INIT                                  0
#define P9OUT_INIT                                  0
#define P9REN_INIT                                  0

/*
    P10.0 =
    P10.1 =
    P10.2 =
    P10.3 =
    P10.4 =
    P10.5 =
    P10.6 =
    P10.7 =
 */

#define PJDIR_INIT                                 0
#define PJSEL_INIT                                 0
#define PJOUT_INIT                                 0


/*
    The LCD is a full 160 segment display, with 2 lines of 8 digits, plus various symbols.
*/
#define LCDCCTL0_INIT                               (LCDFREQ_128 | LCD4MUX | LCDSON | LCDON)
#define LCDCCTL1_INIT                               (0)
#define LCDCPCTL0_INIT                              (0xFFFF)     // Seg 0 -
#define LCDCPCTL1_INIT                              (0xFFFF)     //
//#define LCDCPCTL2_INIT                            (0xFF)       // - Seg 39  
#define LCDCPCTL2_INIT                              (0xC3)       // - Seg 39 LCD Workaround
#define LCDCVCTL0_INIT                              (LCDCPEN)
#define LCDCVCTL1_INIT                              (0)

void setup_gpio(void);
void setup_lcd(void);
void setup_adc(void);

#endif
