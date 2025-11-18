//--------------------------------------------------------------------------
//
//  DMM - Defines Module
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


#define AVERAGING_SAMPLES       4096
#define DIVIDE_NUMBER_SHIFTS    12                              // must be log2(AVERAGING_SAMPLES)

#define DC_A_COEFFICIENT        .4                              // coefficients for DC smoothing filter
#define AC_A_COEFFICIENT        .3

#define OVERFLOW_H               240000                         // Overflow/out-of-range detection limits
#define OVERFLOW_L               -240000

#define SD24B_OSR_RATIO         64                              // ADC oversampling ratio
#define SD24B_CURRENT_PGA_GAIN  4                               // options G1->0,G2-1,G4-2,G8->3,G16->4,G32->5,G64->6
#define SD24B_VOLTAGE_PGA_GAIN  4                               // options G1->0,G2-1,G4-2,G8->3,G16->4,G32->5,G64->6

//Voltage Mode Gain Factor and Offset Constants                                      
#define VOLTAGE_60mV_GAIN               (2.77521E-7)            // Volts per LSB  
#define VOLTAGE_60mV_OFFSET             2218                    // LSBs

#define VOLTAGE_600mV_GAIN              (3.02116E-6)            // Volts per LSB
#define VOLTAGE_600mV_OFFSET            1911                    // LSBs

#define VOLTAGE_6V_GAIN                 (3.03370E-5)            // Volts per LSB
#define VOLTAGE_6V_OFFSET               2892                    // LSBs

#define VOLTAGE_60V_GAIN                (3.02877E-4)            // Volts per LSB
#define VOLTAGE_60V_OFFSET              2400                    // LSBs

//Current Mode Gain Factor and Offset Constants   
#define CURRENT_600uA_GAIN              (2.88383E-9)            // Amps per LSB
#define CURRENT_600uA_OFFSET            2386                    // LSBs

#define CURRENT_60mA_GAIN               (5.48909E-7)            // Amps per LSB
#define CURRENT_60mA_OFFSET              2383                   // LSBs

//Power Mode Gain Factor and Offset Constants 
#define POWER_MODE_6V_GAIN              (3.03358E-5)            // Volts per LSB
#define POWER_MODE_600uA_GAIN           (2.88376E-9)            // Amps per LSB
#define POWER_MODE_6V_OFFSET            2117                    // LSBs
#define POWER_MODE_600uA_OFFSET         2232                    // LSBs

#define POWER_MODE_60V_GAIN             (3.02876E-4)            // Volts per LSB
#define POWER_MODE_60mA_GAIN            (5.48853E-7)            // Amps per LSB
#define POWER_MODE_60V_OFFSET           2186                    // LSBs
#define POWER_MODE_60mA_OFFSET          2205                    // LSBs

// MSP430F6736 Operating Mode
#define MCU_OPERATING_MODE_2                    // 0-1.8V/8MHz, 2-2.2V/16MHz
#define TEST_MODE               0               // enables streaming of samples to serial port

#define FLASH_MAIN_PAGE_SIZE                    512
#define FLASH_INFO_PAGE_SIZE                    128

//EVENT Bits
#define KEY1_EVENT		 		0x01
#define KEY2_EVENT		 		0x02
#define RTC_EVENT				0x04
#define SD24B_EVENT			        0x08

//KEYS
#define KEY_DEBOUNCE_COUNT		        15		                //assume 6.4 mS timer interval

#define SD24B_IFG_REG                           SD24BIFG
#define SD24B_VOLTAGE_IFG_BIT                   SD24IFG0
#define SD24B_CURRENT_IFG_BIT                   SD24IFG1

#define SD24B_IE_REG                            SD24BIE
#define SD24B_VOLTAGE_IE_BIT                    SD24IE0
#define SD24B_CURRENT_IE_BIT                    SD24IE1

#define SD24B_VOLTAGE_ADC_CTL_REG               SD24BCCTL0
#define SD24B_CURRENT_ADC_CTL_REG               SD24BCCTL1
#define SD24B_ADC_EN_BITS                       SD24SCS_4                       // Bits that bring ADC in and out of Group 0 (enabling/disabling ADCs)

// Measurement Modes
#define DC_VOLTAGE                              1
#define AC_VOLTAGE                              2        
#define DC_CURRENT                              3       
#define AC_CURRENT                              4
#define POWER                                   5
#define OFF                                     6

// Voltage Ranges
#define V_60mV                                  1     
#define V_600mV                                 2
#define V_6V                                    3
#define V_60V                                   4

// Current Ranges
#define I_60mA                                  1               
#define I_600uA                                 2

// Power Ranges
#define P_3600uW                                1            
#define P_3600mW                                2

// OTHER
#define BATTERY_CHECK_INTERVAL			2				//minutes

//PLATFORM SPECIFIC--------------------------------------------------------------------------

// KEYS
#define KEY_IE_REG				P2IE                            
#define KEY_IFG_REG				P2IFG
#define KEY_PORT_IN_REG				P2IN
#define KEY1					BIT6
#define KEY2					BIT7
#define ALL_KEY_MASK				0xC0

//POWER MANAGEMENT
#define FRONT_END_EN_REG			P2OUT
#define FRONT_END_EN_BIT		        BIT0
#define BATT_TEST_EN_REG			P2OUT
#define BATT_TEST_EN_BIT			BIT4
#define TEST_EN_REG			        P1OUT
#define TEST_EN_BIT		                BIT7
#define TEST_RAM_CS_REG				P1OUT			        
#define TEST_RAM_CS_BIT				BIT0			        

// DMM and AFE related defines
#define VOLTAGE_RANGE_REG       		P2OUT
#define VOLTAGE_RANGE_MASK                      0x06
#define VOLTAGE_RANGE_MSB       		BIT2
#define VOLTAGE_RANGE_LSB       		BIT1
#define CURRENT_RANGE_REG       		P2OUT
#define CURRENT_RANGE_MASK                      0x08
#define CURRENT_RANGE_LSB       		BIT3

// Port Initialization defines

/*! This defines the speed of USART 1 or USCI 0 */
#define UART_PORT_1_SUPPORT                                 1
#define UART1_BAUD_RATE                                     9600

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

#define P1DIR_INIT                                  (BIT0 | BIT1 | BIT3 | BIT6 | BIT7)
#define P1SEL_INIT                                  (BIT2 | BIT3 |BIT4 | BIT5 | BIT6)
#define P1OUT_INIT                                  (BIT0)
#define P1REN_INIT                                  (0)
#define P1MAP01_INIT                                (0)

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

#define P2DIR_INIT                                  (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)
#define P2SEL_INIT                                  (0)
#define P2OUT_INIT                                  (BIT0 | BIT6 | BIT7)
#define P2REN_INIT                                  (BIT6 | BIT7)
#define P2IES_INIT                                  (BIT6 | BIT7)

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

#define P3DIR_INIT                                  0x0F
#define P3SEL_INIT                                  0
#define P3OUT_INIT                                  (BIT0)
#define P3REN_INIT                                  0

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


// OTHER
#define __inline__ inline

#define DEFL(name, address) __no_init volatile unsigned long int name @ address;
#define DEFLL(name, address) __no_init volatile unsigned long long int name @ address;

#if !defined(DEFXC)
#define DEFXC  volatile unsigned char
#endif
#if !defined(DEFXW)
#define DEFXW  volatile unsigned short
#endif
#if !defined(DEFXA)
#if __REGISTER_MODEL__ == __REGISTER_MODEL_REG20__
#define DEFXA  void __data20 * volatile
#else
#define DEFXA  volatile unsigned short  /* only short access is allowed from C in small memory model */
#endif

#define DEFXL  volatile unsigned long int
#define DEFXLL volatile unsigned long long int

#if defined(__MSP430_HAS_MPY__)  ||  defined(__MSP430_HAS_MPY32__)
/* Byte, 16 bit word and 32 bit word access to the result register of the 16 bit multiplier */

#if defined(__MSP430_HAS_PMM__)
#define MPY_BASE    0x4C0
#else
#define MPY_BASE    0x130
#endif

#define RES16_32_           (MPY_BASE + 10)   /* 16x16 bit result */
__no_init union
{
  DEFXC                               RES16_8[4];
  DEFXW                               RES16_16[2];
  DEFXL                               RES16_32;
} @ (MPY_BASE + 10);

#endif

#if defined(__MSP430_HAS_MPY32__)
/* Byte, 16 bit word, 32 bit word and 64 bit word access to the registers of the 32 bit multiplier */

#if defined(__MSP430_HAS_PMM__)
#define MPY32_BASE_A  0x4D0
#else
#define MPY32_BASE_A  0x140
#endif

#define MPY32_              (MPY32_BASE_A + 0)    /* Multiply Unsigned Operand 1 */
__no_init union
{
  DEFXC                               MPY8[4];
  DEFXW                               MPY16[2];
  DEFXL                               MPY32;
} @ (MPY32_BASE_A + 0);
#endif

#define MPYS32_             (MPY32_BASE_A + 4)    /* Multiply Signed Operand 1 */
__no_init union
{
  DEFXC                               MPYS8[4];
  DEFXW                               MPYS16[2];
  DEFXL                               MPYS32;
} @ (MPY32_BASE_A + 4);

#define MAC32_              (MPY32_BASE_A + 8)    /* MAC Unsigned Operand 1 */
__no_init union
{
  DEFXC                               MAC8[4];
  DEFXW                               MAC16[2];
  DEFXL                               MAC32;
} @ (MPY32_BASE_A + 8);

#define MACS32_             (MPY32_BASE_A + 12)   /* MAC Signed Operand 1 */
__no_init union
{
  DEFXC                               MACS8[4];
  DEFXW                               MACS16[2];
  DEFXL                               MACS32;
} @ (MPY32_BASE_A + 12);

#define OP2_32_             (MPY32_BASE_A + 16)   /* Operand 2 */
__no_init union
{
  DEFXC                               OP2_8[4];
  DEFXW                               OP2_16[2];
  DEFXL                               OP2_32X;
} @ (MPY32_BASE_A + 16);

#define RES64_              (MPY32_BASE_A + 20)   /* 32x32 bit result */
__no_init union
{
  DEFXC                               RES8[8];
  DEFXW                               RES16[4];
  DEFXL                               RES32[2];
  DEFXLL                              RES64;
} @ (MPY32_BASE_A + 20);

#endif

#define SD24BMEM0_32_       (0x0850u)
__no_init union
{
  DEFXC                               SD24BMEM0_8[4];
  DEFXW                               SD24BMEM0_16[2];
  DEFXL                               SD24BMEM0_32;
} @ (0x0850u);

#define SD24BMEM1_32_       (0x0854u)
__no_init union
{
  DEFXC                               SD24BMEM1_8[4];
  DEFXW                               SD24BMEM1_16[2];
  DEFXL                               SD24BMEM1_32;
} @ (0x0854u);

#define SD24BMEM2_32_       (0x0858u)
__no_init union
{
  DEFXC                               SD24BMEM2_8[4];
  DEFXW                               SD24BMEM2_16[2];
  DEFXL                               SD24BMEM2_32;
} @ (0x0858u);


