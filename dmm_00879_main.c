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
#pragma diag_suppress 1531 
#pragma diag_suppress 1538

#include "in430.h"
#include "intrinsics.h"
#include <msp430.h>
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>

#include "dmm_00879_defines.h"
#include "dmm_00879_hw_setup.h"
#include "dmm_00879_main.h"
#include "uart.h"
#include "timer.h"
#include "wdt.h"
#include "globals.h"

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
const uint8_t test_mode = 0;
const uint8_t calibrate = 0;
uint8_t decimal_position = 4;
uint8_t unit = 0;
uint8_t power_mode_cal = 0;

int main(void) {

  // WDT Enable
  WDT_Enable();

  // Hardware Setup
  system_setup();

  // Setup GPIO
  // setup_gpio();

  // Init UART
  uart_init_9600();
  //uart_init_115200();

  // === Boot message ===
  uart_print("****************************************\r\n");
  uart_print("* TI DMM-00879 – External Crystal Test *\r\n");
  uart_print("* 32.768 kHz XT1 → 16.78 MHz MCLK/SMCLK*\r\n");
  uart_print("* UART 9600 baud                       *\r\n");
  uart_print("****************************************\r\n");
  uart_println();

  // Enable Interrupts
  __enable_interrupt();

  // Enable ADC in Voltage Measurement Mode
  // enable_disable_ADCs(DC_VOLTAGE);
  // voltage_settings(dc_voltage_range);
  dc_a_coefficient = DC_A_COEFFICIENT;
  dc_b_coefficient = 1 - dc_a_coefficient;
  ac_a_coefficient = AC_A_COEFFICIENT;
  ac_b_coefficient = 1 - ac_a_coefficient;

  KEY_IE_REG |= ALL_KEY_MASK;

  while (1) {
      dc_voltage_measurement_mode();
      ac_voltage_measurement_mode();
      dc_current_measurement_mode();
      ac_current_measurement_mode();
      power_measurement_mode();
      off();
  }
}

void dc_voltage_measurement_mode_old() {
  measurement_mode = DC_VOLTAGE;
  uart_print_interrupt("\033[2J");  // clear screen
  uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
  uart_print_interrupt("\r\nDC Voltage Measurement Mode\r\n");
  enable_disable_ADCs(DC_VOLTAGE);
  voltage_settings(dc_voltage_range);

  while (1) {
    // Low Power Mode with interrupts
    // _BIS_SR(LPM0_bits + GIE);

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;

        if (test_mode == 1) {
          if (calibrate == 1)
            send_sample(calibration_value_fp);
          else
            send_sample(result_fp);
        }
      }
    }

    // KEY 1 - MODE
    // SETS MEASUREMENT MODE
    else if ((Events & KEY1_EVENT) > 0) {
      Events &= ~KEY1_EVENT; // clear Event bit
      // update_display();
      return;
    }
    
    // KEY2 - RANGE
    // SETS RANGE OF EACH MEASUREMENT MODE
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
      // update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void dc_voltage_measurement_mode__(void) {
    measurement_mode = DC_VOLTAGE;

    uart_print_interrupt("\033[2J\033[H");                     // clear + home in one shot
    uart_print_interrupt("\r\n=== DC VOLTAGE MODE ===\r\n");

    enable_disable_ADCs(DC_VOLTAGE);        // turn on only voltage ADC
    voltage_settings(dc_voltage_range);     // set gain/offset/range GPIOs

    while (1)
    {
        /* --------------------------------------------------------------
           1) ADC conversion finished → process the block of 4096 samples
           -------------------------------------------------------------- */
        if (Events & SD24B_EVENT)
        {
            Events &= ~SD24B_EVENT;

            if (sampling_completed)
            {
                sampling_completed = 0;

                // Optional: stream raw calibrated value in test/cal mode
                if (test_mode)
                {
                    if (calibrate)
                        send_sample(calibration_value_fp);   // raw ADC counts
                    else
                        send_sample(result_fp);              // engineering units
                }

                // Normal UI update (you can add LCD here later)
                uart_print_interrupt("\rDC V: ");
                uart_print_float(result_fp, decimal_position);
                uart_print_interrupt(" V       ");   // overwrite previous line
            }
        }

        /* --------------------------------------------------------------
           2) KEY1 → change measurement mode (exit this function)
           -------------------------------------------------------------- */
        if (Events & KEY1_EVENT)
        {
            Events &= ~KEY1_EVENT;
            uart_print_interrupt("\r\n--- Leaving DC Voltage ---\r\n");
            return;                         // go to next mode in main()
        }

        /* --------------------------------------------------------------
           3) KEY2 → cycle through voltage ranges
           -------------------------------------------------------------- */
        if (Events & KEY2_EVENT)
        {
            Events &= ~KEY2_EVENT;

            switch (dc_voltage_range)
            {
                case V_60mV:  dc_voltage_range = V_600mV; break;
                case V_600mV: dc_voltage_range = V_6V;    break;
                case V_6V:    dc_voltage_range = V_60V;   break;
                case V_60V:   dc_voltage_range = V_60mV;  break;
                default:      dc_voltage_range = V_60V;   break;
            }

            voltage_settings(dc_voltage_range);   // apply new range (GPIO + gain/offset)

            // Immediate feedback so you see it really changed
            uart_print_interrupt("\r\n→ Range: ");
            switch (dc_voltage_range)
            {
                case V_60mV:  uart_print_interrupt("60 mV\r\n");  break;
                case V_600mV: uart_print_interrupt("600 mV\r\n"); break;
                case V_6V:    uart_print_interrupt("6 V\r\n");    break;
                case V_60V:   uart_print_interrupt("60 V\r\n");   break;
            }

            // Reset filter so you don’t get a huge jump from old data
            //last_sampled_value_fp = 0.0f;
        }

        /* --------------------------------------------------------------
           4) Re-enable key interrupts once debounce is finished
           -------------------------------------------------------------- */
        if (key_debounce == 0)
        {
            KEY_IE_REG |= ALL_KEY_MASK;     // P2.6 & P2.7 interrupts on again
        }

        /* --------------------------------------------------------------
           5) Optional: tiny yield so UART TX buffer can empty
               (only needed if you are spamming uart_print a lot)
           -------------------------------------------------------------- */
        __delay_cycles(800);   // ~50 µs at 16 MHz → totally optional
    }
}

void dc_voltage_measurement_mode_auto_mult_line(void) {
    measurement_mode = DC_VOLTAGE;

    uart_print_interrupt("\033[2J\033[H");
    uart_print_interrupt("\r\n=== DC VOLTAGE AUTO RANGE ===\r\n");

    enable_disable_ADCs(DC_VOLTAGE);
    voltage_settings(dc_voltage_range);        // start in whatever range we were in

    // Hysteresis thresholds (in volts) — tuned for the real DMM front-end
    const float upper_threshold[] = { 0.065f, 0.65f,  6.5f,  65.0f };  // > 108% of full scale
    const float lower_threshold[] = { 0.054f, 0.54f,  5.4f,  54.0f };  // < 90% of full scale

    // Add this near your other global variables (usually in dmm_00879_main.c or a globals file)
    uint8_t range_overflow = 0;

    while (1)
    {
        if (Events & SD24B_EVENT)
        {
            Events &= ~SD24B_EVENT;

            if (sampling_completed)
            {
                sampling_completed = 0;

                // ----------------------------------------------------------------
                // 1) AUTO RANGING LOGIC — runs every 4096 samples (~50–100 ms)
                // ----------------------------------------------------------------
                float abs_v = (result_fp < 0) ? -result_fp : result_fp;
                uint8_t current_idx;

                // Map current range to array index (0=60mV, 1=600mV, 2=6V, 3=60V)
                switch (dc_voltage_range)
                {
                    case V_60mV:  current_idx = 0; break;
                    case V_600mV: current_idx = 1; break;
                    case V_6V:    current_idx = 2; break;
                    case V_60V:   current_idx = 3; break;
                    default:      current_idx = 3; break;
                }

                uint8_t new_range = dc_voltage_range;   // default = stay

                // Too high → go up one range (with overflow protection)
                if (abs_v > upper_threshold[current_idx] || range_overflow > 0)
                {
                    if (current_idx < 3)
                    {
                        new_range = (current_idx == 0) ? V_600mV :
                                    (current_idx == 1) ? V_6V    : V_60V;
                    }
                }
                // Too low → go down one range (but only if we gain resolution)
                else if (abs_v < lower_threshold[current_idx] && current_idx > 0)
                {
                    new_range = (current_idx == 1) ? V_60mV :
                                (current_idx == 2) ? V_600mV : V_6V;
                }

                // Apply new range if changed
                if (new_range != dc_voltage_range)
                {
                    dc_voltage_range = new_range;
                    voltage_settings(dc_voltage_range);

                    // Immediate visual feedback
                    uart_print_interrupt("\r\n→ AUTO → ");
                    switch (dc_voltage_range)
                    {
                        case V_60mV:  uart_print_interrupt("60 mV");  break;
                        case V_600mV: uart_print_interrupt("600 mV"); break;
                        case V_6V:    uart_print_interrupt("6 V");    break;
                        case V_60V:   uart_print_interrupt("60 V");   break;
                    }
                    uart_print_interrupt("\r");
                }

                // ----------------------------------------------------------------
                // 2) Normal display update
                // ----------------------------------------------------------------
                uart_print_interrupt("\rDC V: ");
                uart_print_float(result_fp, decimal_position);
                uart_print_interrupt(" V   → ");

                switch (dc_voltage_range)
                {
                    case V_60mV:  uart_print_interrupt("60 mV");  break;
                    case V_600mV: uart_print_interrupt("600 mV"); break;
                    case V_6V:    uart_print_interrupt("6 V");    break;
                    case V_60V:   uart_print_interrupt("60 V");   break;
                }
            }
        }

        // ----------------------------------------------------------------
        // 3) Manual exit with KEY1
        // ----------------------------------------------------------------
        if (Events & KEY1_EVENT)
        {
            Events &= ~KEY1_EVENT;
            uart_print_interrupt("\r\n--- Leaving DC Voltage ---\r\n");
            return;
        }

        // Re-enable key interrupts
        if (key_debounce == 0)
            KEY_IE_REG |= ALL_KEY_MASK;

        // Optional: remove this delay once your ADC is slowed down
        // __delay_cycles(800);
    }
}

void dc_voltage_measurement_mode_(void) {
    measurement_mode = DC_VOLTAGE;

    uart_print_interrupt("\033[2J\033[H");
    uart_print_interrupt("\r\n=== DC VOLTAGE AUTO RANGE ===\r\n");

    enable_disable_ADCs(DC_VOLTAGE);
    voltage_settings(dc_voltage_range);        // start in whatever range we were in

    // Hysteresis thresholds (in volts) — tuned for the real DMM front-end
    const float upper_threshold[] = { 0.065f, 0.65f,  6.5f,  65.0f };  // > 108% of full scale
    const float lower_threshold[] = { 0.054f, 0.54f,  5.4f,  54.0f };  // < 90% of full scale

    // Add this near your other global variables (usually in dmm_00879_main.c or a globals file)
    const uint8_t range_overflow = 0;

    while (1)
    {
        if (Events & SD24B_EVENT)
        {
            Events &= ~SD24B_EVENT;

            if (sampling_completed)
            {
                sampling_completed = 0;
                
                // ----------------------------------------------------------------
                // 1) AUTO RANGING LOGIC — runs every 4096 samples (~50–100 ms)
                // ----------------------------------------------------------------
                float abs_v = (result_fp < 0) ? -result_fp : result_fp;
                uint8_t current_idx;

                // Map current range to array index (0=60mV, 1=600mV, 2=6V, 3=60V)
                switch (dc_voltage_range)
                {
                    case V_60mV:  current_idx = 0; break;
                    case V_600mV: current_idx = 1; break;
                    case V_6V:    current_idx = 2; break;
                    case V_60V:   current_idx = 3; break;
                    default:      current_idx = 3; break;
                }

                uint8_t new_range = dc_voltage_range;   // default = stay

                // Too high → go up one range (with overflow protection)
                if (abs_v > upper_threshold[current_idx] || range_overflow > 0)
                {
                    if (current_idx < 3)
                    {
                        new_range = (current_idx == 0) ? V_600mV :
                                    (current_idx == 1) ? V_6V    : V_60V;
                    }
                }
                // Too low → go down one range (but only if we gain resolution)
                else if (abs_v < lower_threshold[current_idx] && current_idx > 0)
                {
                    new_range = (current_idx == 1) ? V_60mV :
                                (current_idx == 2) ? V_600mV : V_6V;
                }

                // Apply new range if changed
                if (new_range != dc_voltage_range)
                {
                    dc_voltage_range = new_range;
                    voltage_settings(dc_voltage_range);
                }
                  
                // ----------------------------------------------------------------
                // 2) Normal display update
                // ----------------------------------------------------------------
                static uint8_t display_init = 1;

                if (display_init)
                {
                    uart_print_interrupt("\033[2J\033[H");   // clear screen once
                    uart_print_interrupt("=== DC VOLTAGE AUTO RANGE ===\r\n");
                    display_init = 0;
                }

                // Clear line + rewrite everything
                uart_print_interrupt("\r\033[2K");         // ← MAGIC: clear entire line

                uart_print_interrupt("DC V: ");
                uart_print_float(result_fp, decimal_position);
                uart_print_interrupt(" V   →  ");

                switch (dc_voltage_range)
                {
                    case V_60mV:  uart_print_interrupt(" 60 mV  "); break;
                    case V_600mV: uart_print_interrupt("600 mV  "); break;
                    case V_6V:    uart_print_interrupt("  6 V   "); break;
                    case V_60V:   uart_print_interrupt(" 60 V   "); break;
                }
            }
        }

        // ----------------------------------------------------------------
        // 3) Manual exit with KEY1
        // ----------------------------------------------------------------
        if (Events & KEY1_EVENT)
        {
            Events &= ~KEY1_EVENT;
            uart_print_interrupt("\r\n--- Leaving DC Voltage ---\r\n");
            return;
        }

        // Re-enable key interrupts
        if (key_debounce == 0)
            KEY_IE_REG |= ALL_KEY_MASK;

        // Optional: remove this delay once your ADC is slowed down
        // __delay_cycles(800);
    }
}

void dc_voltage_measurement_mode(void)
{
    measurement_mode = DC_VOLTAGE;
    /* ========== FORCE CLEAN START IN 6V RANGE ========== */
    enable_disable_ADCs(DC_VOLTAGE);
    //voltage_settings(dc_voltage_range);

    voltage_settings(V_60mV);

    /* Clear screen and title */
    uart_print_interrupt("\033[2J\033[H");
    uart_print_interrupt("=== DC VOLTAGE AUTO RANGE ===\r\n");

    /* Hysteresis thresholds */
    const float upper[] = { 0.063f, 0.650f,  6.5f,  65.0f };
    const float lower[] = { 0.00f, 0.300f,  1.0f,  7.0f };

    uint8_t range_overflow = 0;

    while (1)
    {
        if (Events & SD24B_EVENT)
        {
            Events &= ~SD24B_EVENT;

            if (sampling_completed)
            {
                sampling_completed = 0;
                if (range_overflow) range_overflow--;
                
                __disable_interrupt();
                int32_t current_counts = counts; // Atomic read of the 32-bit variable
                __enable_interrupt();

                uart_print_signed_integer_interrupt(current_counts);
                uart_println_interrupt();
                // /* ——— AUTO RANGING ——— */
                // float abs_v = (result_fp < 0) ? -result_fp : result_fp;

                // uint8_t idx = 1;   // default = 6V
                // switch (dc_voltage_range)
                // {
                //     case V_60mV:  idx = 0; break;
                //     case V_600mV: idx = 1; break;
                //     case V_6V:    idx = 2; break;
                //     case V_60V:   idx = 3; break;
                // }

                // uint8_t new_range = dc_voltage_range;

                // if (range_overflow || abs_v > upper[idx])
                // {
                //     if (idx < 3)
                //         new_range = (idx == 0) ? V_600mV : (idx == 1) ? V_6V : V_60V;
                // }
                // else if (abs_v < lower[idx] && idx > 0)
                // {
                //     new_range = (idx == 1) ? V_60mV : (idx == 2) ? V_600mV : V_6V;
                // }

                // if (new_range != dc_voltage_range)
                // {
                //     dc_voltage_range = new_range;
                //     voltage_settings(dc_voltage_range);
                // }

                /* ——— ONE CLEAN LINE DISPLAY ——— */
                //uart_print_interrupt("\r\033[2KDC V: ");
                uart_print_float(result_fp, decimal_position);
                uart_println_interrupt();
                //uart_print_interrupt(" V  →  ");

                // switch (dc_voltage_range)
                // {
                //     case V_60mV:  uart_print_interrupt(" 60 mV  "); break;
                //     case V_600mV: uart_print_interrupt("600 mV  "); break;
                //     case V_6V:    uart_print_interrupt("  6 V   "); break;
                //     case V_60V:   uart_print_interrupt(" 60 V   "); break;
                // }
            }
        }

        if (Events & KEY1_EVENT)
        {
            Events &= ~KEY1_EVENT;
            uart_print_interrupt("\r\n--- Leaving DC Voltage ---\r\n");
            return;
        }

        if (key_debounce == 0)
            KEY_IE_REG |= ALL_KEY_MASK;
    }
}

void ac_voltage_measurement_mode(void) {
  measurement_mode = AC_VOLTAGE;
  uart_print_interrupt("\033[2J");  // clear screen
  uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
  uart_print_interrupt("\r\nAC Voltage Measurement Mode\r\n");
  enable_disable_ADCs(AC_VOLTAGE);
  voltage_settings(ac_voltage_range);

  while (1) {
    //_BIS_SR(LPM0_bits + GIE);
    //__no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        // update_display();

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
      //update_display();
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
      //update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void dc_current_measurement_mode() {
  measurement_mode = DC_CURRENT;
  uart_print_interrupt("\033[2J");  // clear screen
  uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
  uart_print_interrupt("\r\nDC Current Measurement Mode\r\n");
  //enable_disable_ADCs(DC_CURRENT);
  current_settings(dc_current_range);

  while (1) {
    //_BIS_SR(LPM0_bits);
    //__no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        // update_display();

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
      // update_display();
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
      // update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void ac_current_measurement_mode(void) {
  measurement_mode = AC_CURRENT;
  uart_print_interrupt("\033[2J");  // clear screen
  uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
  uart_print_interrupt("\r\nAC Current Measurement Mode\r\n");
  enable_disable_ADCs(AC_CURRENT);
  current_settings(ac_current_range);

  while (1) {
    //_BIS_SR(LPM0_bits);
    //__no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        //update_display();

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
      //update_display();
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
      //update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void power_measurement_mode(void) {
  measurement_mode = POWER;
  uart_print_interrupt("\033[2J");  // clear screen
  uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
  uart_print_interrupt("\r\nPower Measurement Mode\r\n");
  //enable_disable_ADCs(POWER);
  power_settings(power_range);

  while (1) {
    //_BIS_SR(LPM0_bits);
    //__no_operation();

    if ((Events & SD24B_EVENT) > 0) {
      Events &= ~SD24B_EVENT;
      if (sampling_completed == 1) {
        sampling_completed = 0;
        // update_display();

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
      //update_display();
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
      //update_display();
    } else {
      if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;
    }
  }
}

void off() {
  volatile static uint8_t stop = 0;
  uart_print_interrupt("\r\nPowering Down Measurement Mode\r\n");

  SD24BCTL0 &= ~SD24REFS; // disable reference
  VOLTAGE_RANGE_REG &=
      ~VOLTAGE_RANGE_MASK; // set range bits = 0 to prevent power leakage path
  CURRENT_RANGE_REG &=
      ~CURRENT_RANGE_MASK; // set range bits = 0 to prevent power leakage path
  FRONT_END_EN_REG &= ~FRONT_END_EN_BIT;
  enable_disable_ADCs(OFF);

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

  FRONT_END_EN_REG |= FRONT_END_EN_BIT;
  Events &= ~KEY1_EVENT;
  
}

void off_new(void) {
    measurement_mode = OFF;
    uart_print_interrupt("\033[2J");  // clear screen
    uart_print_interrupt("\033[H");   // cursor to home (row 1, col 1)
    uart_print_interrupt("\r\nOFF Mode (ADC halted)\r\n");
    enable_disable_ADCs(OFF);

    SD24BIE  = 0;
    SD24BIFG = 0;

    if (Events & KEY1_EVENT)
    {
        Events &= ~KEY1_EVENT;
        uart_print_interrupt("\r\n--- Leaving DC Voltage ---\r\n");
        return;
    }

    if (key_debounce == 0)
        KEY_IE_REG |= ALL_KEY_MASK;

    // just return immediately; main() can decide what mode to enter next
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
    //uart_print_interrupt("60mV Range Selected\033[K\r");
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_LSB;
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_MSB;
    active_voltage_offset = VOLTAGE_60mV_OFFSET;
    active_gain = VOLTAGE_60mV_GAIN;
    decimal_position = 4;
    unit = 1; // milli
    break;
  case V_600mV:
    //uart_print_interrupt("600mV Range Selected\033[K\r");
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_LSB;
    active_voltage_offset = VOLTAGE_600mV_OFFSET;
    active_gain = VOLTAGE_600mV_GAIN;
    decimal_position = 3;
    unit = 1; // milli
    break;
  case V_6V:
    //uart_print_interrupt("6V Range Selected\033[K\r");
    VOLTAGE_RANGE_REG |= VOLTAGE_RANGE_MSB;
    VOLTAGE_RANGE_REG &= ~VOLTAGE_RANGE_LSB;
    active_voltage_offset = VOLTAGE_6V_OFFSET;
    active_gain = VOLTAGE_6V_GAIN;
    decimal_position = 5;
    unit = 0; // none
    break;
  case V_60V:
    //uart_print_interrupt("60V Range Selected\033[K\r");
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
    uart_print_interrupt("600uA Range Selected\033[K\r");
    CURRENT_RANGE_REG |= CURRENT_RANGE_LSB;
    active_current_offset = CURRENT_600uA_OFFSET;
    active_gain = CURRENT_600uA_GAIN;
    decimal_position = 3;
    unit = 2; // micro
    break;
  case I_60mA:
    uart_print_interrupt("60mA Range Selected\033[K\r");
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
    uart_print_interrupt("3600uW Range Selected\033[K\r");
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
    uart_print_interrupt("3600mW Range Selected\033[K\r");
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

// void WDT_Enable(void) {
//   WDTCTL = WDTPW + WDTSSEL_2 + WDTTMSEL +
//            WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
//   SFRIE1 |= WDTIE;
// }

// void WDT_Disable(void) {
//   WDTCTL = WDTPW + WDTHOLD + WDTSSEL_2 + WDTTMSEL +
//            WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
//   SFRIE1 &= ~WDTIE;
// }

/*
// Enables P1.7 to drive LDO enable high
// No longer needed as RS232 is removed
void test_interface_power_up() {
  P1DIR = P1DIR_INIT;
  P1SEL = P1SEL_INIT;
  P1OUT = P1OUT_INIT | BIT7;
}

// Disables P1.7 to pull LDO enable low
// No longer needed as RS232 is removed
void test_interface_power_down() {
  P1SEL = 0;
  P1OUT = 0;
  P1DIR = 0xFF;
}
*/
