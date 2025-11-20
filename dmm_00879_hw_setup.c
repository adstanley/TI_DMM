/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//--------------------------------------------------------------------------
//
//  DMM - Hardware Set-up
//
//-------------------------------------------------------------------------
//
#include "dmm_00879_defines.h"
#include "dmm_00879_main.h"
#include "hal_PMM.h"
#include "lcd.h"
#include <msp430f6736.h>
#include <stdint.h>
#include "clock.h"

void system_setup(void) {
  // WDTCTL = WDTPW + WDTHOLD + WDTSSEL_2 + WDTTMSEL +
  //          WDTIS_7; // Configure WDT for debounce, VLOCLK source, interval
  //                   // timer, 6.4mS per tick

  // UCSCTL1 = 0;
  // UCSCTL1 |= DCORSEL_5; // Target 16MHz for MCLK
  // // UCSCTL1 |= DCORSEL_4;				// Target 16MHz for MCLK
  // UCSCTL3 |= SELREF_2; // FLL ref clock to RFOCLK
  // UCSCTL4 = 0;
  // UCSCTL4 |= SELA_1 + SELS_3 + SELM_3; // ACLK=VLOCK, SMCLK=DC0CLK, MCLK=DCOCLK
  // UCSCTL5 |= DIVS_2 + DIVS_1;          // ACLK/1, SMCLK/8, MCLK/1
  // // UCSCTL5 |= DIVS_2;              // ACLK/1, SMCLK/4, MCLK/1
  // UCSCTL6 = 0x010D;
  // UCSCTL2 = 511; // Set FLLN to 511 (/512)
  // // UCSCTL2 = 255;					// Set FLLN to 511
  // // (/512)
  // UCSCTL2 |= FLLD_0; // Set FLLD to divide by 1


  // SetVCore(2);
  Clock_Init_16MHz_XT1();
  
  P1OUT = P1OUT_INIT;
  P1DIR = P1DIR_INIT;
  P1SEL = P1SEL_INIT;
  P1REN = P1REN_INIT;
  P1MAP01 = P1MAP01_INIT;

  P2OUT = P2OUT_INIT;
  P2DIR = P2DIR_INIT;
  P2SEL = P2SEL_INIT;
  P2REN = P2REN_INIT;
  P2IES = P2IES_INIT;

  P3OUT = (BIT2 + BIT1);
  P3DIR = P3DIR_INIT;
  P3SEL = P3SEL_INIT;
  P3REN = P3REN_INIT;

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

  P8OUT = P8OUT_INIT;
  P8DIR = P8DIR_INIT;
  P8SEL = P8SEL_INIT;
  P8REN = P8REN_INIT;

  P9OUT = P9OUT_INIT;
  P9DIR = P9DIR_INIT;
  P9SEL = P9SEL_INIT;
  P9REN = P9REN_INIT;

  PJOUT = PJOUT_INIT;
  PJDIR = PJDIR_INIT;
  PJSEL = PJSEL_INIT;

  LCDCCTL0 = LCDDIV_2 | LCDPRE_3 | LCD4MUX | LCDON | LCDSSEL;

  // Charge pump generated internally at 2.96V, external bias (V2-V4) generation
  // Internal reference for charge pump
  LCDCVCTL = LCDCPEN | VLCD_2_84;
  REFCTL0 &= ~REFMSTR;

  LCDCPCTL0 = 0xFFFF; // Select LCD Segments 4-5
  LCDCPCTL1 = 0xFFFF;
  LCDCPCTL2 = 0xFFC3;  // Changed for LCD workaround;
  LCDCMEMCTL = 0xFFF2; // clear LCD memory
  LCDCBLKCTL = 0x00E6;

  // AFE Setup

  SD24BCTL1 &= ~SD24GRP0SC;
  SD24BCTL0 = SD24SSEL__SMCLK // Based on SMCLK = 2MHz
              | SD24REFS;     // Use internal reference

  // Configure analog front-end channel 1 - Current
  SD24BINCTL1 = SD24B_CURRENT_PGA_GAIN << 3; // Set gain for channel 1
  SD24BCCTL1 = SD24DF_1;                     // right-aligned, 2's complement
  SD24BOSR1 = SD24B_OSR_RATIO - 1;           // Set oversampling ratio
  SD24BPRE1 = 0;

  // Configure analog front-end channel 0 - Voltage
  SD24BINCTL0 = SD24B_VOLTAGE_PGA_GAIN << 3; // Set gain for channel 0 (V)
  SD24BCCTL0 = SD24DF_1;           // Set right-aligned, 2's complement
  SD24BOSR0 = SD24B_OSR_RATIO - 1; // Set oversampling ratio
  SD24BPRE0 = 0;

  SD24BCTL1 |= SD24GRP0SC; // SD16SC

  SD24B_IE_REG |= SD24B_VOLTAGE_IE_BIT;

  // UART Configuration

  UCA1CTL1 |= UCSWRST;  // **Put state machine in reset**
  UCA1CTL1 |= UCSSEL_2; // SMCLK

  // UCA1BR0 = 13;                                               // 8MHz 9600
  // (see User's Guide)
  // UCA1BR1 = 0;                                                // 8MHz 9600
  // UCA1MCTLW =  0xA0 + 0x00 + UCOS16;                          // Modln
  // UCBRSx=0, UCBRFx=10, over sampling

  UCA1BRW = 13;
  UCA1MCTLW = 0xB6A1;

  UCA1CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

  UCA1IFG &= ~UCTXIFG;
  UCA1IFG &= ~UCRXIFG;

  //  Power Supervisor Control Settings
  PMMCTL0_H = 0xA5;
  SVSMIO = 0;
  SVSMHCTL &= ~(SVMHE + SVSHE); // Disable High side SVS
  SVSMLCTL &= ~(SVMLE + SVSLE); // Disable Low side SVS
  PMMCTL0_H = 0;

  FRONT_END_EN_REG |= FRONT_END_EN_BIT;
  test_interface_power_down();

  _EINT();
}
