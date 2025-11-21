#ifndef __LCD_H
#define __LCD_H

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

#include "dmm_00879_main.h"
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


void update_display();

#endif
