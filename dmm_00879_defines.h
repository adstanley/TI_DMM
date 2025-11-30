/*==============================================================================
  DMM - Defines Module+
  
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
==============================================================================*/

#ifndef __DMM_00879_DEFINES_H
#define __DMM_00879_DEFINES_H

//==============================================================================
// Signal Processing & Filtering Constants
//==============================================================================

/* * Number of ADC samples to accumulate before processing.
 * 4096 samples allows for significant noise reduction via oversampling.
 */
#define AVERAGING_SAMPLES 4096

/* * Bit shift count for division. 
 * 2^12 = 4096. Shifting right by 12 is equivalent to dividing by 4096 
 * but is much faster on the MSP430 CPU than a hardware/software divide.
 * must be log2(AVERAGING_SAMPLES)
 */
#define DIVIDE_NUMBER_SHIFTS 12 

/* * Coefficients for First-Order IIR (Infinite Impulse Response) filters.
 * Used for smoothing the display output (Software Low Pass Filter).
 * Formula usually looks like: Output = (Input * A) + (PrevOutput * (1-A))
 */
#define DC_A_COEFFICIENT .4 // coefficients for DC smoothing filter
#define AC_A_COEFFICIENT .3

/* * Software limits for the ADC readings. 
 * Even though the 24-bit ADC can go higher (approx +/- 8 million), 
 * the DMM logic clamps or detects overflow at +/- 240,000 counts 
 * (likely for a 24,000 count display logic).
 */
#define OVERFLOW_H 240000 // Overflow/out-of-range detection limits
#define OVERFLOW_L -240000

//==============================================================================
// SD24_B (Sigma-Delta ADC) Configuration
//==============================================================================

/* * Oversampling Ratio (OSR) setting for the SD24BOSR register.
 * Higher OSR = Better resolution/SNR, but slower sampling rate.
 */
#define SD24B_OSR_RATIO 64 // ADC oversampling ratio

/* * Programmable Gain Amplifier (PGA) settings.
 * Based on the comment G16->4, this sets the ADC internal gain to 16x.
 * This allows measurement of smaller signals utilizing the full range of the ADC.
 * options G1->0,G2-1,G4-2,G8->3,G16->4,G32->5,G64->6
 */
#define SD24B_CURRENT_PGA_GAIN 4
#define SD24B_VOLTAGE_PGA_GAIN 4

//==============================================================================
// Calibration Constants (y = mx + b)
// Used to convert raw ADC counts to real-world units (Volts/Amps/Watts)
// real_voltage = (raw_adc_value - OFFSET) × GAIN
//==============================================================================

// --- Voltage Mode Calibration ---
#define VOLTAGE_60mV_GAIN (2.77521E-7) // Multiplier for 60mV range // Volts per LSB
#define VOLTAGE_60mV_OFFSET 2218       // Zero-offset correction // LSBs

#define VOLTAGE_600mV_GAIN (2*3.02116E-6) // Volts per LSB
#define VOLTAGE_600mV_OFFSET 1911       // LSBs

#define VOLTAGE_6V_GAIN (3.03370E-5) // Volts per LSB
//#define VOLTAGE_6V_OFFSET 2892       // LSBs
#define VOLTAGE_6V_OFFSET 0

#define VOLTAGE_60V_GAIN (3.02877E-4) // Volts per LSB
#define VOLTAGE_60V_OFFSET 2400       // LSBs

// --- Current Mode Calibration ---
// Current Mode Gain Factor and Offset Constants
#define CURRENT_600uA_GAIN (2.88383E-9) // Amps per LSB
#define CURRENT_600uA_OFFSET 2386       // LSBs

#define CURRENT_60mA_GAIN (5.48909E-7) // Amps per LSB
#define CURRENT_60mA_OFFSET 2383       // LSBs

// --- Power Mode Calibration ---
// Power Mode Gain Factor and Offset Constants
#define POWER_MODE_6V_GAIN (3.03358E-5)    // Volts per LSB
#define POWER_MODE_600uA_GAIN (2.88376E-9) // Amps per LSB
#define POWER_MODE_6V_OFFSET 2117          // LSBs
#define POWER_MODE_600uA_OFFSET 2232       // LSBs

#define POWER_MODE_60V_GAIN (3.02876E-4)  // Volts per LSB
#define POWER_MODE_60mA_GAIN (5.48853E-7) // Amps per LSB
#define POWER_MODE_60V_OFFSET 2186        // LSBs
#define POWER_MODE_60mA_OFFSET 2205       // LSBs

//==============================================================================
// System Configuration
//==============================================================================
// MSP430F6736 Operating Mode

/* * PMM (Power Management Module) setting.
 * Mode 2 usually corresponds to higher core voltage (Vcore level 2) 
 * required to run the MCU at higher frequencies (e.g., 16MHz or 20MHz).
 */
#define MCU_OPERATING_MODE_2 // 0-1.8V/8MHz, 2-2.2V/16MHz

/* * Debug Flag. 
 * If 1, code likely dumps raw ADC samples to UART for analysis.
 */
#define TEST_MODE 0          // enables streaming of samples to serial port

// MSP430 Flash Memory organization constants
#define FLASH_MAIN_PAGE_SIZE 512
#define FLASH_INFO_PAGE_SIZE 128

//==============================================================================
// Application State Machine Definitions
//==============================================================================

// Event Flags (Bitmasks) for the main system loop
#define KEY1_EVENT 0x01 // Button 1 pressed
#define KEY2_EVENT 0x02 // Button 2 pressed
#define RTC_EVENT 0x04  // Real-Time Clock interrupt (timing update)
#define SD24B_EVENT 0x08 // Sigma-Delta 24-bit ADC event

// Key Debouncing
#define KEY_DEBOUNCE_COUNT 15 // Number of ticks to wait, assume 6.4 mS timer interval

//==============================================================================
// Hardware Mapping: SD24_B ADC Registers
// Maps generic names to the specific MSP430F6736 header definitions
//==============================================================================

// Interrupt Flag Registers
#define SD24B_IFG_REG SD24BIFG
#define SD24B_VOLTAGE_IFG_BIT SD24IFG0
#define SD24B_CURRENT_IFG_BIT SD24IFG1

// Interrupt Enable Registers
#define SD24B_IE_REG SD24BIE
#define SD24B_VOLTAGE_IE_BIT SD24IE0
#define SD24B_CURRENT_IE_BIT SD24IE1

// Channel Control Registers
#define SD24B_VOLTAGE_ADC_CTL_REG SD24BCCTL0
#define SD24B_CURRENT_ADC_CTL_REG SD24BCCTL1

/* * Start Conversion Select Bits.
 * SD24SCS_4 (Binary 100) usually selects a specific Group trigger 
 * or external trigger source depending on the specific family user guide.
 */
#define SD24B_ADC_EN_BITS SD24SCS_4

//==============================================================================
// DMM Measurement State Enums
//==============================================================================

// Measurement Modes
#define DC_VOLTAGE 1
#define AC_VOLTAGE 2
#define DC_CURRENT 3
#define AC_CURRENT 4
#define POWER 5
#define OFF 6
#define OHMS 7

// Range Enums (Used for switch/case logic in auto-ranging)
// Voltage Ranges
#define V_60mV 1
#define V_600mV 2
#define V_6V 3
#define V_60V 4

// Current Ranges
#define I_60mA 1
#define I_600uA 2

// Power Ranges
#define P_3600uW 1
#define P_3600mW 2

// Resistance Ranges
#define R_1O 1
#define R_10K 2
#define R_100K 3
#define R_1M 4

//==============================================================================
// PLATFORM SPECIFIC I/O MAPPING
//==============================================================================

// --- User Input (Keys) ---
// Mapped to Port 2, Pins 6 and 7
#define KEY_IE_REG P2IE         // Port 2 Interrupt Enable
#define KEY_IFG_REG P2IFG       // Port 2 Interrupt Flag
#define KEY_PORT_IN_REG P2IN    // Port 2 Input Register
#define KEY1 BIT6               // P2.6
#define KEY2 BIT7               // P2.7
#define ALL_KEY_MASK 0xC0       // Mask for both keys (BIT6 | BIT7)

// --- Power & Test Pins ---
#define FRONT_END_EN_REG P2OUT  // Port 2 Output Register
#define FRONT_END_EN_BIT BIT0   // P2.0 - Controls Analog Front End Power
#define BATT_TEST_EN_REG P2OUT  
#define BATT_TEST_EN_BIT BIT4   // P2.4 - Enables Battery Test Load
#define TEST_EN_REG P1OUT
#define TEST_EN_BIT BIT7        // P1.7 - Test Enable Signal
#define TEST_RAM_CS_REG P1OUT
#define TEST_RAM_CS_BIT BIT0    // P1.0 - Chip Select for External RAM/Flash

// --- Analog Front End (AFE) Range Switching ---
// Controls external Muxes/Relays via GPIO to change measurement ranges
#define VOLTAGE_RANGE_REG P2OUT
#define VOLTAGE_RANGE_MASK 0x06 // Controls P2.1 and P2.2
#define VOLTAGE_RANGE_MSB BIT2
#define VOLTAGE_RANGE_LSB BIT1

#define CURRENT_RANGE_REG P2OUT
#define CURRENT_RANGE_MASK 0x08 // Controls P2.3
#define CURRENT_RANGE_LSB BIT3


//==============================================================================
//  Hardware Multiplier 32-bit – FINAL correct signed MAC for AC RMS (CCS)
//==============================================================================

/* * MPY32 Hardware Multiplier Access.
 * The MSP430F6736 has a dedicated 32-bit multiplier peripheral.
 * MACS32L is the "Multiply Accumulate Signed 32-bit Low Word" register.
 * Writing to this register initiates the multiplication of the value written
 * with the value currently in OP2, and adds the result to the Result register.
 */
#define MACS32   MACS32L          // Accesses address 0x04D0 (MACS32L)

// Operand 2 – Writing here sets the second number for the multiplication.
#define OP2_32X   OP2L

/* * 64-bit Result Reconstruction.
 * The multiplier result spans 4 16-bit registers: RES0, RES1, RES2, RES3.
 * This macro casts them to 64-bit (unsigned long long) and shifts them 
 * into position to create a single 64-bit result variable in C.
 * Essential for accumulating squares (V^2 or I^2) for RMS measurements without overflow.
 */
// #define RES64   ( (unsigned long long)RES3 << 48 | \
//                   (unsigned long long)RES2 << 32 | \
//                   (unsigned long long)RES1 << 16 | \
//                   RES0 )

#define RES64   (*((volatile int64_t*)&RESLO))

#endif
