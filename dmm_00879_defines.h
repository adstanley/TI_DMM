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

#define AVERAGING_SAMPLES 4096
#define DIVIDE_NUMBER_SHIFTS 12 // must be log2(AVERAGING_SAMPLES)

#define DC_A_COEFFICIENT .4 // coefficients for DC smoothing filter
#define AC_A_COEFFICIENT .3

#define OVERFLOW_H 240000 // Overflow/out-of-range detection limits
#define OVERFLOW_L -240000

#define SD24B_OSR_RATIO 64 // ADC oversampling ratio

// options G1->0,G2-1,G4-2,G8->3,G16->4,G32->5,G64->6
#define SD24B_CURRENT_PGA_GAIN 4

// options G1->0,G2-1,G4-2,G8->3,G16->4,G32->5,G64->6
#define SD24B_VOLTAGE_PGA_GAIN 4

// Voltage Mode Gain Factor and Offset Constants
#define VOLTAGE_60mV_GAIN (2.77521E-7) // Volts per LSB
#define VOLTAGE_60mV_OFFSET 2218       // LSBs

#define VOLTAGE_600mV_GAIN (3.02116E-6) // Volts per LSB
#define VOLTAGE_600mV_OFFSET 1911       // LSBs

#define VOLTAGE_6V_GAIN (3.03370E-5) // Volts per LSB
#define VOLTAGE_6V_OFFSET 2892       // LSBs

#define VOLTAGE_60V_GAIN (3.02877E-4) // Volts per LSB
#define VOLTAGE_60V_OFFSET 2400       // LSBs

// Current Mode Gain Factor and Offset Constants
#define CURRENT_600uA_GAIN (2.88383E-9) // Amps per LSB
#define CURRENT_600uA_OFFSET 2386       // LSBs

#define CURRENT_60mA_GAIN (5.48909E-7) // Amps per LSB
#define CURRENT_60mA_OFFSET 2383       // LSBs

// Power Mode Gain Factor and Offset Constants
#define POWER_MODE_6V_GAIN (3.03358E-5)    // Volts per LSB
#define POWER_MODE_600uA_GAIN (2.88376E-9) // Amps per LSB
#define POWER_MODE_6V_OFFSET 2117          // LSBs
#define POWER_MODE_600uA_OFFSET 2232       // LSBs

#define POWER_MODE_60V_GAIN (3.02876E-4)  // Volts per LSB
#define POWER_MODE_60mA_GAIN (5.48853E-7) // Amps per LSB
#define POWER_MODE_60V_OFFSET 2186        // LSBs
#define POWER_MODE_60mA_OFFSET 2205       // LSBs

// MSP430F6736 Operating Mode
#define MCU_OPERATING_MODE_2 // 0-1.8V/8MHz, 2-2.2V/16MHz
#define TEST_MODE 0          // enables streaming of samples to serial port

#define FLASH_MAIN_PAGE_SIZE 512
#define FLASH_INFO_PAGE_SIZE 128

// EVENT Bits
#define KEY1_EVENT 0x01
#define KEY2_EVENT 0x02
#define RTC_EVENT 0x04
#define SD24B_EVENT 0x08

// KEYS
#define KEY_DEBOUNCE_COUNT 15 // assume 6.4 mS timer interval

#define SD24B_IFG_REG SD24BIFG
#define SD24B_VOLTAGE_IFG_BIT SD24IFG0
#define SD24B_CURRENT_IFG_BIT SD24IFG1

#define SD24B_IE_REG SD24BIE
#define SD24B_VOLTAGE_IE_BIT SD24IE0
#define SD24B_CURRENT_IE_BIT SD24IE1

#define SD24B_VOLTAGE_ADC_CTL_REG SD24BCCTL0
#define SD24B_CURRENT_ADC_CTL_REG SD24BCCTL1

// Bits that bring ADC in and out of Group 0 (enabling/disabling ADCs)
#define SD24B_ADC_EN_BITS SD24SCS_4

// Measurement Modes
#define DC_VOLTAGE 1
#define AC_VOLTAGE 2
#define DC_CURRENT 3
#define AC_CURRENT 4
#define POWER 5
#define OFF 6

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

// OTHER
#define BATTERY_CHECK_INTERVAL 2 // minutes

// PLATFORM SPECIFIC
//==============================================================================

// KEYS
#define KEY_IE_REG P2IE
#define KEY_IFG_REG P2IFG
#define KEY_PORT_IN_REG P2IN
#define KEY1 BIT6
#define KEY2 BIT7
#define ALL_KEY_MASK 0xC0

// POWER MANAGEMENT
#define FRONT_END_EN_REG P2OUT
#define FRONT_END_EN_BIT BIT0
#define BATT_TEST_EN_REG P2OUT
#define BATT_TEST_EN_BIT BIT4
#define TEST_EN_REG P1OUT
#define TEST_EN_BIT BIT7
#define TEST_RAM_CS_REG P1OUT
#define TEST_RAM_CS_BIT BIT0

// DMM and AFE related defines
#define VOLTAGE_RANGE_REG P2OUT
#define VOLTAGE_RANGE_MASK 0x06
#define VOLTAGE_RANGE_MSB BIT2
#define VOLTAGE_RANGE_LSB BIT1
#define CURRENT_RANGE_REG P2OUT
#define CURRENT_RANGE_MASK 0x08
#define CURRENT_RANGE_LSB BIT3

//==============================================================================
// Port Initialization defines
//==============================================================================

/*! This defines the speed of USART 1 or USCI 0 */
#define UART_PORT_1_SUPPORT 1
#define UART1_BAUD_RATE 9600

/*
    P1.0 =                        - Memory CS
    P1.1 =                        - NC
    P1.2 =                        - SOMI
    P1.3 =                        - SIMO
    P1.4 =                        - UART RX
    P1.5 =                        - UART TX
    P1.6 =                        - SCLK
    P1.7 =                        - TEST_EN
 */

#define P1DIR_INIT (BIT0 | BIT1 | BIT3 | BIT6 | BIT7)
#define P1SEL_INIT (BIT2 | BIT3 | BIT4 | BIT5 | BIT6)
#define P1OUT_INIT (BIT0)
#define P1REN_INIT (0)
#define P1MAP01_INIT (0)

/*
    P2.0 =                        - SCL for EEPROM
    P2.1 =                        - SDA for EEPROM
    P2.2 =                        - A2SOMI
    P2.3 =                        - A2SIMO
    P2.4 =                        - A2CS
    P2.5 =                        - A2CLK
    P2.6 =                        - Key1
    P2.7 =                        - Key2
 */

#define P2DIR_INIT (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)
#define P2SEL_INIT (0)
#define P2OUT_INIT (BIT0 | BIT6 | BIT7)
#define P2REN_INIT (BIT6 | BIT7)
#define P2IES_INIT (BIT6 | BIT7)

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

#define P3DIR_INIT 0x0F
#define P3SEL_INIT 0
#define P3OUT_INIT (BIT0)
#define P3REN_INIT 0

//==============================================================================
//  Hardware Multiplier 32-bit – FINAL correct signed MAC for AC RMS (CCS)
//==============================================================================

// Signed 32-bit Multiply-Accumulate operand 1 (this is the one we need!)
#define MACS32   MACS32L          // code writes only low 32 bits → perfect

// Operand 2 – also only low 32 bits are used
#define OP2_32X   OP2L

// 64-bit signed result – built cleanly from the four official result registers
#define RES64   ( (unsigned long long)RES3 << 48 | \
                  (unsigned long long)RES2 << 32 | \
                  (unsigned long long)RES1 << 16 | \
                  RES0 )

#endif
