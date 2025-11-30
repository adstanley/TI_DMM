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
#include <msp430f6736.h>
#include <stdint.h>
#include "clock.h"

void system_setup(void) {
  // timer, 6.4mS per tick
  // Configure WDT for debounce, VLOCLK source, interval
  // WDTCTL = WDTPW + WDTHOLD + WDTSSEL_2 + WDTTMSEL + WDTIS_7; 

  // Initialize Clock
  Clock_Init_16MHz(); 

  // AFE Setup
  SD24BCTL1 &= ~SD24GRP0SC;

  SD24BCTL0 = SD24SSEL__SMCLK    // source = SMCLK
            | SD24REFS           // internal 1.2V reference
            | SD24PDIV_2         // pre-scaler = /2
            | SD24DIV4;          // primary divider = /4
                                 // → 16 MHz / 2 / 4 = 2MHz to match ref design

  // Configure analog front-end channel 1 - Current
  SD24BINCTL1 = SD24B_CURRENT_PGA_GAIN << 3;; // Set gain for channel 1
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
  // UCA1CTL1 |= UCSWRST;  // **Put state machine in reset**
  // UCA1CTL1 |= UCSSEL_2; // SMCLK

  // UCA1BR0 = 13;                                               // 8MHz 9600
  // (see User's Guide)
  // UCA1BR1 = 0;                                                // 8MHz 9600
  // UCA1MCTLW =  0xA0 + 0x00 + UCOS16;                          // Modln
  // UCBRSx=0, UCBRFx=10, over sampling

  // UCA1BRW = 13;
  // UCA1MCTLW = 0xB6A1;

  // UCA1CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

  // UCA1IFG &= ~UCTXIFG;
  // UCA1IFG &= ~UCRXIFG;

  //  Power Supervisor Control Settings
  PMMCTL0_H = 0xA5;
  SVSMIO = 0;
  SVSMHCTL &= ~(SVMHE + SVSHE); // Disable High side SVS
  SVSMLCTL &= ~(SVMLE + SVSLE); // Disable Low side SVS
  PMMCTL0_H = 0;

  FRONT_END_EN_REG |= FRONT_END_EN_BIT;
}
