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
//  DMM - Main Module
//
//--------------------------------------------------------------------------
//

#define __MAIN_PROGRAM__
#include <msp430.h>
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>


#include "dmm_00879_defines.h"
#include "dmm_00879_hw_setup.h"
#include "dmm_00879_main.h"


// Variables used by other modules
uint8_t sampling_completed;
int32_t sampled_value;
uint8_t number_is_negative;
uint8_t measurement_mode = DC_VOLTAGE;
uint8_t Events;
uint8_t key_debounce;
int32_t active_voltage_offset = VOLTAGE_60V_OFFSET;
int32_t active_current_offset;
float dc_a_coefficient = 0;
float dc_b_coefficient = 0;
float ac_a_coefficient = 0;
float ac_b_coefficient = 0;

int32_t display_value;
volatile unsigned int LCD_reg;

float active_voltage_gain;
float active_current_gain;
float active_power_gain;
float active_gain = VOLTAGE_60V_GAIN;
float displayed_result_fp;
float result_fp;
float displayed_result_factor;
float calibration_value_fp;

uint8_t dc_voltage_range = V_60V;
uint8_t dc_current_range = I_60mA;
uint8_t ac_voltage_range = V_60V;
uint8_t ac_current_range = I_60mA;
uint8_t power_range = P_3600mW;
uint8_t test_mode = 0;
uint8_t calibrate = 0;
uint8_t decimal_position = 4;
uint8_t unit = 0;
uint8_t power_mode_cal = 0;

const unsigned char LCD_Char_Map[] = {
    BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7,               // '0' or 'O'
    BIT5 | BIT6,                                           // '1' or 'I'
    BIT0 | BIT1 | BIT3 | BIT4 | BIT6 | BIT7,               // '2' or 'Z'
    BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7,               // '3'
    BIT0 | BIT1 | BIT2 | BIT5 | BIT6,                      // '4' or 'y'
    BIT0 | BIT1 | BIT2 | BIT4 | BIT5 | BIT7,               // '5' or 'S'
    BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT7,        // '6' or 'b'
    BIT5 | BIT6 | BIT7,                                    // '7'
    BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7, // '8' or 'B'
    BIT0 | BIT1 | BIT2 | BIT4 | BIT5 | BIT6 | BIT7,        // '9' or 'g'
};

int __low_level_init(void) // low level init function
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  return 1;
}

void main(void) {
  static uint8_t temp;

  system_setup();
  update_display();

  enable_disable_ADCs(DC_VOLTAGE);
  voltage_settings(dc_voltage_range);
  dc_a_coefficient = DC_A_COEFFICIENT;
  dc_b_coefficient = 1 - dc_a_coefficient;
  ac_a_coefficient = AC_A_COEFFICIENT;
  ac_b_coefficient = 1 - ac_a_coefficient;

  temp = KEY_PORT_IN_REG & KEY1;
  if ((temp == 0) | (TEST_MODE == 1)) {
    test_mode = 1;
    test_interface_power_up();
    LCDM20 |= 0x0F;
  }

  temp = KEY_PORT_IN_REG & KEY2;
  if (temp == 0) {
    test_mode = 1;
    LCDM20 |= 0x0F;
    test_interface_power_up();
    calibrate = 1;
    active_gain = 1;
    active_voltage_offset = 0;
    active_current_offset = 0;
    LCDM20 |= 0x30;
  }

  KEY_IE_REG |= ALL_KEY_MASK;

  while (1) {
    if (calibrate == 0) {
      power_mode_cal = 0;
      dc_voltage_measurement_mode();
      ac_voltage_measurement_mode();
      dc_current_measurement_mode();
      ac_current_measurement_mode();
      power_measurement_mode();
      off();
    } else {
      power_mode_cal = 0;
      dc_voltage_measurement_mode();
      power_mode_cal = 1;
      dc_voltage_measurement_mode();
      power_mode_cal = 0;
      dc_current_measurement_mode();
      power_mode_cal = 1;
      dc_current_measurement_mode();
    }
  }
}

void dc_voltage_measurement_mode() {
  measurement_mode = DC_VOLTAGE;
  enable_disable_ADCs(DC_VOLTAGE);
  voltage_settings(dc_voltage_range);

  while (1) {
    _BIS_SR(LPM0_bits);
    __no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        update_display();

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    ////// KEY1    ///////////////////   Return to main measurement mode loop
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      update_display();
      return;
    }
    ////// KEY2 ///////////////////////////  SETS RANGE OF EACH MEASUREMENT MODE
    /////////////////////
    else if ((Events & KEY2_EVENT) > 0) {
      Events &= ~KEY2_EVENT;

      switch (dc_voltage_range) {
      case V_60mV:
        dc_voltage_range = V_600mV;
        break;
      case V_600mV:
        dc_voltage_range = V_6V;
        break;
      case V_6V:
        dc_voltage_range = V_60V;
        break;
      case V_60V:
        dc_voltage_range = V_60mV;
        break;
      default:
        break;
      }

      voltage_settings(dc_voltage_range);
      update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void ac_voltage_measurement_mode(void) {
  measurement_mode = AC_VOLTAGE;
  enable_disable_ADCs(AC_VOLTAGE);
  voltage_settings(ac_voltage_range);

  while (1) {
    _BIS_SR(LPM0_bits);
    __no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        update_display();

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    ////// KEY1    ///////////////////   Return to main measurement mode loop
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      update_display();
      return;
    }
    ////// KEY2 ///////////////////////////  SETS RANGE OF EACH MEASUREMENT MODE
    /////////////////////
    else if ((Events & KEY2_EVENT) > 0) {
      Events &= ~KEY2_EVENT;

      switch (ac_voltage_range) {
      case V_60mV:
        ac_voltage_range = V_600mV;
        break;
      case V_600mV:
        ac_voltage_range = V_6V;
        break;
      case V_6V:
        ac_voltage_range = V_60V;
        break;
      case V_60V:
        ac_voltage_range = V_60mV;
        break;
      default:
        break;
      }

      voltage_settings(ac_voltage_range);
      update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void dc_current_measurement_mode() {
  measurement_mode = DC_CURRENT;
  enable_disable_ADCs(DC_CURRENT);
  current_settings(dc_current_range);

  while (1) {
    _BIS_SR(LPM0_bits);
    __no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        update_display();

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    ////// KEY1    ///////////////////   Return to main measurement mode loop
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      update_display();
      return;
    }
    ////// KEY2 ///////////////////////////  SETS RANGE OF EACH MEASUREMENT MODE
    /////////////////////
    else if ((Events & KEY2_EVENT) > 0) {
      Events &= ~KEY2_EVENT;

      switch (dc_current_range) {
      case I_60mA:
        dc_current_range = I_600uA;
        break;
      case I_600uA:
        dc_current_range = I_60mA;
        break;
      default:
        break;
      }

      current_settings(dc_current_range);
      update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void ac_current_measurement_mode(void) {
  measurement_mode = AC_CURRENT;
  enable_disable_ADCs(AC_CURRENT);
  current_settings(ac_current_range);

  while (1) {
    _BIS_SR(LPM0_bits);
    __no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        update_display();

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    ////// KEY1    ///////////////////   Return to main measurement mode loop
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      update_display();
      return;
    }
    ////// KEY2 ///////////////////////////  SETS RANGE OF EACH MEASUREMENT MODE
    /////////////////////
    else if ((Events & KEY2_EVENT) > 0) {
      Events &= ~KEY2_EVENT;

      switch (ac_current_range) {
      case I_60mA:
        ac_current_range = I_600uA;
        break;
      case I_600uA:
        ac_current_range = I_60mA;
        break;
      default:
        break;
      }

      current_settings(ac_current_range);
      update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void power_measurement_mode(void) {
  measurement_mode = POWER;
  enable_disable_ADCs(POWER);
  power_settings(power_range);

  while (1) {
    _BIS_SR(LPM0_bits);
    __no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        update_display();

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    ////// KEY1    ///////////////////   Return to main measurement mode loop
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      update_display();
      return;
    }
    ////// KEY2 ///////////////////////////  SETS RANGE OF EACH MEASUREMENT MODE
    /////////////////////
    else if ((Events & KEY2_EVENT) > 0) {
      Events &= ~KEY2_EVENT;

      switch (power_range) {
      case P_3600uW:
        power_range = P_3600mW;
        break;
      case P_3600mW:
        power_range = P_3600uW;
        break;
      default:
        break;
      }

      power_settings(power_range);
      update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void off() {
  volatile static uint8_t stop = 0;

  SD24BCTL0 &= ~SD24REFS; // disable reference
  VOLTAGE_RANGE_REG &=
      ~VOLTAGE_RANGE_MASK; // set range bits = 0 to prevent power leakage path
  CURRENT_RANGE_REG &=
      ~CURRENT_RANGE_MASK; // set range bits = 0 to prevent power leakage path
  FRONT_END_EN_REG &= ~FRONT_END_EN_BIT;
  enable_disable_ADCs(OFF);
  LCDCCTL0 &= ~LCDON;

  test_interface_power_down();
  P3DIR |= (BIT6 + BIT7);
  P4DIR |= (BIT0 + BIT1);

  stop = 0;
  while (stop == 0) {
    if (key_debounce > 1)
      stop = 0;
    else
      stop = 1;
  }

  KEY_IFG_REG &= ~ALL_KEY_MASK;
  KEY_IE_REG |= KEY1;
  KEY_IE_REG &= ~KEY2;
  WDT_Disable();

  __bis_SR_register(LPM4_bits + GIE);
  __no_operation();

  P3DIR &= ~(BIT6 + BIT7);
  P4DIR &= ~(BIT0 + BIT1);
  SD24BCTL0 |= SD24REFS; // disable reference

  FRONT_END_EN_REG |= FRONT_END_EN_BIT;

  if (test_mode == 1)
    test_interface_power_up();

  Events &= ~KEY1_EVENT;

  LCDCCTL0 |= LCDON;
}

void enable_disable_ADCs(uint8_t i) {
  SD24BCTL1 &= ~SD24GRP0SC; // disable group ADCs
  if ((i == DC_VOLTAGE) || (i == AC_VOLTAGE)) {
    SD24B_VOLTAGE_ADC_CTL_REG |=
        SD24B_ADC_EN_BITS; // enable voltage ADC (add to Group0)

    if (power_mode_cal == 1)
      SD24B_CURRENT_ADC_CTL_REG |=
          SD24B_ADC_EN_BITS; // enable current ADC (add to Group0)
    else
      SD24B_CURRENT_ADC_CTL_REG &=
          ~SD24B_ADC_EN_BITS; // disable current ADC (remove from Group0)

    SD24B_IE_REG |= SD24B_VOLTAGE_IE_BIT;  // enable voltage ADC interrupts
    SD24B_IE_REG &= ~SD24B_CURRENT_IE_BIT; // disable current ADC interrupts
  } else if ((i == DC_CURRENT) || (i == AC_CURRENT)) {
    if (power_mode_cal == 1)
      SD24B_VOLTAGE_ADC_CTL_REG |=
          SD24B_ADC_EN_BITS; // enable voltage ADC (add to Group0)
    else
      SD24B_VOLTAGE_ADC_CTL_REG &=
          ~SD24B_ADC_EN_BITS; // disable voltage ADC (remove from Group0)

    SD24B_CURRENT_ADC_CTL_REG |=
        SD24B_ADC_EN_BITS;                 // enable current ADC (add to Group0)
    SD24B_IE_REG &= ~SD24B_VOLTAGE_IE_BIT; // disable voltage ADC interrupts
    SD24B_IE_REG |= SD24B_CURRENT_IE_BIT;  // enable current ADC interrupts
  } else if (i == POWER) {
    SD24B_VOLTAGE_ADC_CTL_REG |=
        SD24B_ADC_EN_BITS; // enable voltage ADC (add to Group0)
    SD24B_CURRENT_ADC_CTL_REG |=
        SD24B_ADC_EN_BITS;                // enable current ADC (add to Group0)
    SD24B_IE_REG |= SD24B_VOLTAGE_IE_BIT; // enable voltage ADC interrupts
    SD24B_IE_REG |= SD24B_CURRENT_IE_BIT; // enable current ADC interrupts
  } else if (i == OFF) {
    SD24B_VOLTAGE_ADC_CTL_REG &=
        ~SD24B_ADC_EN_BITS; // disable voltage ADC (remove from Group0)
    SD24B_CURRENT_ADC_CTL_REG &=
        ~SD24B_ADC_EN_BITS; // disable current ADC (remove from Group0)
    SD24B_IE_REG &= ~SD24B_VOLTAGE_IE_BIT; // disable voltage ADC interrupts
    SD24B_IE_REG &= ~SD24B_CURRENT_IE_BIT; // disable current ADC interrupts
  }

  SD24BCTL1 |= SD24GRP0SC; // enable group0 ADCs
}

void voltage_settings(uint8_t i) {
  switch (i) {
  case V_60mV:
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_LSB;
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_MSB;
    active_voltage_offset = VOLTAGE_60mV_OFFSET;
    active_gain = VOLTAGE_60mV_GAIN;
    decimal_position = 4;
    unit = 1; // milli
    break;
  case V_600mV:
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_LSB;
    active_voltage_offset = VOLTAGE_600mV_OFFSET;
    active_gain = VOLTAGE_600mV_GAIN;
    decimal_position = 3;
    unit = 1; // milli
    break;
  case V_6V:
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_LSB;
    active_voltage_offset = VOLTAGE_6V_OFFSET;
    active_gain = VOLTAGE_6V_GAIN;
    decimal_position = 5;
    unit = 0; // none
    break;
  case V_60V:
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_LSB;
    active_voltage_offset = VOLTAGE_60V_OFFSET;
    active_gain = VOLTAGE_60V_GAIN;
    decimal_position = 4;
    unit = 0; // none
    break;
  default:
    break;
  }

  set_display_result_factor();
}

void current_settings(uint8_t i) {
  switch (i) {
  case I_600uA:
    CURRENT_RANGE_REG |= CURRENT_RANGE_LSB;
    active_current_offset = CURRENT_600uA_OFFSET;
    active_gain = CURRENT_600uA_GAIN;
    decimal_position = 3;
    unit = 2; // micro
    break;
  case I_60mA:
    CURRENT_RANGE_REG &= ~CURRENT_RANGE_LSB;
    active_current_offset = CURRENT_60mA_OFFSET;
    active_gain = CURRENT_60mA_GAIN;
    decimal_position = 4;
    unit = 1; // milli
    break;
  default:
    break;
  }

  set_display_result_factor();
}

void power_settings(uint8_t i) {
  switch (i) {
  case P_3600uW:
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_LSB;
    CURRENT_RANGE_REG |= CURRENT_RANGE_LSB;
    active_voltage_offset = POWER_MODE_6V_OFFSET;
    active_current_offset = POWER_MODE_600uA_OFFSET;
    active_gain = POWER_MODE_6V_GAIN * POWER_MODE_600uA_GAIN;
    decimal_position = 2;
    unit = 2; // micro
    break;
  case P_3600mW:
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_LSB;
    CURRENT_RANGE_REG &= ~CURRENT_RANGE_LSB;
    active_voltage_offset = POWER_MODE_60V_OFFSET;
    active_current_offset = POWER_MODE_60mA_OFFSET;
    active_gain = POWER_MODE_60V_GAIN * POWER_MODE_60mA_GAIN;
    decimal_position = 2;
    unit = 1; // milli
    break;
  default:
    break;
  }

  set_display_result_factor();
}

void WDT_Enable(void) {
  WDTCTL = WDTPW + WDTSSEL_2 + WDTTMSEL +
           WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
  SFRIE1 |= WDTIE;
}

void WDT_Disable(void) {
  WDTCTL = WDTPW + WDTHOLD + WDTSSEL_2 + WDTTMSEL +
           WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
  SFRIE1 &= ~WDTIE;
}

void test_interface_power_down() {
  P1SEL = 0;
  P1OUT = 0;
  P1DIR = 0xFF;
}

void test_interface_power_up() {
  P1DIR = P1DIR_INIT;
  P1SEL = P1SEL_INIT;
  P1OUT = P1OUT_INIT | BIT7;
}

void send_sample(float sample) {
  static uint32_t sample_int;

  sample_int = *((uint32_t *)&sample);

  while ((UCA1STATW & UCBUSY) > 0)
    ; // header byte to sync byte boundary
  UCA1TXBUF = 0xAA;
  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = 0x55; // header byte to sync byte boundary
  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 24) & 0xFF);
  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 16) & 0xFF);
  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int >> 8) & 0xFF);
  while ((UCA1STATW & UCBUSY) > 0)
    ;
  UCA1TXBUF = (uint8_t)(((uint32_t)sample_int) & 0xFF);
}

void set_display_result_factor() {
  displayed_result_factor = 1;

  switch (unit) {
  case 0: // none
    break;
  case 1: // milli
    displayed_result_factor = displayed_result_factor * 1000;
    break;
  case 2: // micro
    displayed_result_factor = displayed_result_factor * 1000000;
    break;
  default:
    break;
  }

  switch (decimal_position) {
  case 2:
    displayed_result_factor = displayed_result_factor * 10;
    break;
  case 3:
    displayed_result_factor = displayed_result_factor * 100;
    break;
  case 4:
    displayed_result_factor = displayed_result_factor * 1000;
    break;
  case 5:
    displayed_result_factor = displayed_result_factor * 10000;
    break;
  default:
    break;
  }
}

void update_display() {

  LCDM1 = 0;  // main line, digit 6 number
  LCDM2 = 0;  // main line, digit 6 starburst + sign
  LCDM3 = 0;  // main line, digit 5 number
  LCDM4 = 0;  // main line, digit 5 starburst + decimal point
  LCDM5 = 0;  // main line, digit 4 number
  LCDM6 = 0;  // main line, digit 4 starburst + decimal point
  LCDM7 = 0;  // main line, digit 3 number
  LCDM8 = 0;  // main line, digit 3 starburst + decimal point
  LCDM9 = 0;  // main line, digit 2 number
  LCDM10 = 0; // main line, digit 2 starburst + decimal point

  switch (measurement_mode) {
  case DC_VOLTAGE:
    if (calibrate == 0) {
      LCDM14 = 0xC7;
      LCDM16 = 0xC9;
      LCDM17 = 0xCF;
    } else {
      if (power_mode_cal == 0) {
        LCDM14 = 0xC7;
        LCDM16 = 0x00;
        LCDM17 = 0xC9;
      } else {
        LCDM14 = 0xC7;
        LCDM16 = 0xEC;
        LCDM17 = 0xC9;
      }
    }
    break;
  case AC_VOLTAGE:
    LCDM14 = 0xC7;
    LCDM16 = 0xC9;
    LCDM17 = 0xEE;
    break;
  case DC_CURRENT:
    if (calibrate == 0) {
      LCDM14 = 0xEE;
      LCDM16 = 0xC9;
      LCDM17 = 0xCF;
    } else {
      if (power_mode_cal == 0) {
        LCDM14 = 0xEE;
        LCDM16 = 0x00;
        LCDM17 = 0xC9;
      } else {
        LCDM14 = 0xEE;
        LCDM16 = 0xEC;
        LCDM17 = 0xC9;
      }
    }
    break;
  case AC_CURRENT:
    LCDM14 = 0xEE;
    LCDM16 = 0xC9;
    LCDM17 = 0xEE;
    break;
  case POWER:
    LCDM14 = 0xEC;
    LCDM16 = 0;
    LCDM17 = 0;
    break;
  default:
    break;
  }

  switch (unit) {
  case 0: // none
    LCDM11 = 0x0;
    LCDM12 = 0x0;
    break;
  case 1: // milli
    LCDM11 = 0x44;
    LCDM12 = 0xa0;
    break;
  case 2: // micro
    LCDM11 = 0x47;
    LCDM12 = 0x00;
    break;
  default:
    break;
  }

  switch (decimal_position) {
  case 2:
    LCDM8 |= 0x0001;
    break;
  case 3:
    LCDM6 |= 0x0001;
    break;
  case 4:
    LCDM4 |= 0x0001;
    break;
  case 5:
    LCDM2 |= 0x0001;
    break;
  default:
    break;
  }

  display_value = (int32_t)(displayed_result_fp);
  if (display_value < 0) {
    LCDM2 |= 0x04;
    display_value = ~display_value + 1;
  }

  LCDM9 = LCD_Char_Map[display_value % 10];

  display_value /= 10;
  LCDM7 = LCD_Char_Map[display_value % 10];

  display_value /= 10;
  LCDM5 = LCD_Char_Map[display_value % 10];

  display_value /= 10;
  LCDM3 = LCD_Char_Map[display_value % 10];

  display_value /= 10;
  LCDM1 = LCD_Char_Map[display_value % 10];
}
