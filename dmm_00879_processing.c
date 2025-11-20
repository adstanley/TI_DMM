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
//   DMM - Background Module
//
//--------------------------------------------------------------------------
//
#include "dmm_00879_defines.h"
#include "dmm_00879_main.h"
#include <math.h>
#include <msp430.h>
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>


extern uint8_t key_debounce;
extern uint8_t sampling_completed;
extern int32_t sampled_value;
extern uint8_t number_is_negative;
extern uint8_t measurement_mode;
extern uint8_t Events;
extern int32_t active_voltage_offset;
extern int32_t active_current_offset;
extern float displayed_result_fp;
extern float result_fp;
extern float displayed_result_factor;
extern float calibration_value_fp;
extern float active_gain;
extern float dc_a_coefficient;
extern float dc_b_coefficient;
extern float ac_a_coefficient;
extern float ac_b_coefficient;

uint16_t sample_count = 0;
uint16_t sample_count2 = 0;
int32_t operand1;
int32_t operand2;
int32_t dc_acc_sampled_value;
int64_t acc_sampled_value;
int64_t ac_acc_sampled_value;
float last_sampled_value_fp = 0;
float sampled_value_fp = 0;
uint8_t range_overflow = 0;

/*---------------------------------------------------------------------------
  This is the main interrupt routine where the main signal processing is done
  ---------------------------------------------------------------------------*/

#define SD24BMEM0_32 SD24BMEMH0
#define SD24BMEM1_32 SD24BMEMH1

#pragma vector = SD24B_VECTOR
__interrupt void sd24b_adc_interrupt(void) {
  Events |= SD24B_EVENT; // set SD24B ADC Event bit
  sample_count++;

  switch (measurement_mode) {
  case DC_VOLTAGE: // Process DC Voltage mode
    operand1 = SD24BMEM0_32;
    if ((operand1 > 245760) || (operand1 < -245760))
      range_overflow = 8;
    dc_acc_sampled_value += operand1;
    break;
  case DC_CURRENT: // Process DC Current mode
    operand1 = SD24BMEM1_32;
    if ((operand1 > 245760) || (operand1 < -245760))
      range_overflow = 8;
    dc_acc_sampled_value += operand1;
    break;
  case AC_VOLTAGE: // Process AC_Voltage mode
    operand1 = SD24BMEM0_32;
    if ((operand1 > 245760) || (operand1 < -245760))
      range_overflow = 8;
    operand1 -= active_voltage_offset;
    MACS32 = operand1;
    OP2_32X = operand1;
    __delay_cycles(6);
    break;
  case AC_CURRENT: // Process AC_Current mode
    operand1 = SD24BMEM1_32;
    if ((operand1 > 245760) || (operand1 < -245760))
      range_overflow = 8;
    operand1 -= active_current_offset;
    MACS32 = operand1;
    OP2_32X = operand1;
    __delay_cycles(6);
    break;
  case POWER: // Process Power mode
    while ((SD24B_IFG_REG & SD24B_CURRENT_IFG_BIT) == 0) {
      ;
    }
    operand1 = SD24BMEM0_32;
    operand2 = SD24BMEM1_32;
    if ((operand1 > OVERFLOW_H) || (operand1 < OVERFLOW_L) ||
        (operand2 > OVERFLOW_H) || (operand2 < OVERFLOW_L))
      range_overflow = 8;
    operand1 -= active_voltage_offset;
    operand2 -= active_current_offset;
    MACS32 = operand1;
    OP2_32X = operand2;
    __delay_cycles(6);
    break;
  default:
    break;
  }

  if (sample_count >=
      AVERAGING_SAMPLES) // Complete computations after number of averaging
                         // samples has been reached
  {
    if ((measurement_mode == AC_VOLTAGE) ||
        (measurement_mode ==
         AC_CURRENT)) // calculate square root, subtract offset, multiply by
                      // gain factor, run through AC smoothing filter
    {
      ac_acc_sampled_value = RES64;
      ac_acc_sampled_value >>= DIVIDE_NUMBER_SHIFTS;
      sampled_value_fp = (float)ac_acc_sampled_value;
      sampled_value_fp = (sqrtf(sampled_value_fp) * active_gain);
      last_sampled_value_fp =
          ac_b_coefficient * last_sampled_value_fp +
          ac_a_coefficient * sampled_value_fp; // compute smoothing filter
    } else if (measurement_mode == POWER) // multiply by gain factor and run
                                          // through AC smoothing filter
    {
      ac_acc_sampled_value = (RES64 >> DIVIDE_NUMBER_SHIFTS);
      sampled_value_fp = (float)ac_acc_sampled_value;
      sampled_value_fp = sampled_value_fp * active_gain;
      last_sampled_value_fp = ac_b_coefficient * last_sampled_value_fp +
                              ac_a_coefficient * sampled_value_fp;
    } else {
      sampled_value_fp = (float)(dc_acc_sampled_value >>= DIVIDE_NUMBER_SHIFTS);
      calibration_value_fp = sampled_value_fp;
      if (measurement_mode == DC_VOLTAGE)
        sampled_value_fp =
            (sampled_value_fp - active_voltage_offset) *
            active_gain; // substract offset and multiply by gain factor and run
                         // through DC smoothing filter
      else
        sampled_value_fp =
            (sampled_value_fp - active_current_offset) * active_gain;

      last_sampled_value_fp = dc_b_coefficient * last_sampled_value_fp +
                              dc_a_coefficient * sampled_value_fp;
    }

    result_fp = last_sampled_value_fp;
    displayed_result_fp = result_fp * displayed_result_factor;
    sampling_completed = 1;
    dc_acc_sampled_value = 0;
    //RES64 = 0;
    sample_count = 0;

    if (range_overflow > 0) {
      LCDCBLKCTL |= BIT1; // Blink LCD to indicate Overflow/out-of-range conditi
      range_overflow--;
    } else
      LCDCBLKCTL &= ~BIT1;
  }

  SD24BIFG &= ~SD24IFG0;
  SD24BIFG &= ~SD24IFG1;

  _BIC_SR_IRQ(LPM0_bits); // Exit LPM0
}

#pragma vector = PORT2_VECTOR // Range and Mode Button Interrupts
__interrupt void port2_interrupt(void) {
  if (KEY_IFG_REG & KEY1) {
    Events |= KEY1_EVENT;
    KEY_IFG_REG &= ~KEY1; // Clear Key1 IFG and IE
    KEY_IE_REG &= ~KEY1;
    WDT_Enable(); // Enable Watchdog Timer for debounce
    key_debounce = KEY_DEBOUNCE_COUNT;
    SFRIE1 |= WDTIE;
  }

  if ((KEY_IFG_REG & KEY2) > 0) {
    Events |= KEY2_EVENT;
    KEY_IFG_REG &= ~KEY2; // Clear Key2 IFG and IE
    KEY_IE_REG &= ~KEY2;
    WDT_Enable(); // Enable Watchdog Timer for debounce
    key_debounce = KEY_DEBOUNCE_COUNT;
    SFRIE1 |= WDTIE;
  }

  last_sampled_value_fp =
      0; // re-initialize ADC averaging filter due to range or mode change
  sampled_value_fp = 0;

  _BIC_SR_IRQ(LPM0_bits); // Exit LPM0
}

#pragma vector = WDT_VECTOR // used for key debounce
__interrupt void wdt_interrupt(void) {
  volatile char all_key_status;

  if (key_debounce > 1) // key debounce in progress
  {
    all_key_status = (KEY_PORT_IN_REG & ALL_KEY_MASK);

    if (all_key_status == (KEY1 + KEY2)) // as key is not down
      key_debounce--;                    // so decrement counter
    else                                 // key is down
      key_debounce = KEY_DEBOUNCE_COUNT; // so reset counter
  }

  else // key debounce is done
  {
    KEY_IFG_REG &= ~(KEY1 + KEY2); // so reset all IFGs
    KEY_IE_REG |= KEY1 + KEY2;     // re-enable all keyboard interrupts
    WDT_Disable();                 // disable WDT interrupts
  }

  SFRIFG1 &= ~WDTIFG;
  _BIC_SR_IRQ(LPM0_bits); // Exit LPM0
}

// #pragma vector = TIMER0_A0_VECTOR
// __interrupt void battery_check_interrupt(void) {
//   {
//     ;
//   }
// }

#pragma vector = ADC10_VECTOR
__interrupt void adc10_interrupt(void) { ; }
