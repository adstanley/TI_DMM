#include <stdint.h>
#include <stdlib.h>

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = ADC12_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC12_VECTOR)))
#endif
    void
    ADC12_ISR(void) {

  static uint16_t index = 0;
  static uint8_t memory[Num_of_Results] = {
      ADC12_B_MEMORY_0,  ADC12_B_MEMORY_1, ADC12_B_MEMORY_2,  ADC12_B_MEMORY_3,
      ADC12_B_MEMORY_4,  ADC12_B_MEMORY_5, ADC12_B_MEMORY_6,  ADC12_B_MEMORY_7,
      ADC12_B_MEMORY_8,  ADC12_B_MEMORY_9, ADC12_B_MEMORY_10, ADC12_B_MEMORY_11,
      ADC12_B_MEMORY_12,
  };
  switch (__even_in_range(ADC12IV, 42)) {
  case 0:
    break; // Vector  0:  No interrupt
  case 2:
    break; // Vector  2:  ADC12BMEMx Overflow
  case 4:
    break; // Vector  4:  Conversion time overflow
  case 6:
    break; // Vector  6:  ADC12BHI
  case 8:
    break; // Vector  8:  ADC12BLO
  case 10:
    break; // Vector 10:  ADC12BIN
  case 12:
    break; // Clear CPUOFF bit from 0(SR)
  case 14:
    break; // Vector 14:  ADC12BMEM1
  case 16:
    break; // Vector 16:  ADC12BMEM2
  case 18:
    break; // Vector 18:  ADC12BMEM3
  case 20:
    break; // Vector 20:  ADC12BMEM4
  case 22:
    break; // Vector 22:  ADC12BMEM5
  case 24:
    break; // Vector 24:  ADC12BMEM6
  case 26:
    break; // Vector 26:  ADC12BMEM7
  case 28:
    break; // Vector 28:  ADC12BMEM8
  case 30:
    break; // Vector 30:  ADC12BMEM9
  case 32:
    break; // Vector 32:  ADC12BMEM10
  case 34:
    break; // Vector 34:  ADC12BMEM11
  case 36:
    for (index = 0; index < Num_of_Results; index++) {
      results[index] = ADC12_B_getResults(ADC12_B_BASE, memory[index]);
    }
    break; // Vector 36:  ADC12BMEM12
  case 38:
    break; // Vector 38:  ADC12BMEM13
  case 40:
    break; // Vector 40:  ADC12BMEM14
  case 42:
    break; // Vector 42:  ADC12BMEM15
  default:
    break;
  }
}

void example(void) {
  WDTCTL = WDTPW + WDTHOLD;       // Stop WDT
  P1DIR |= 0x01;                  // P1.0 output
  TACTL = TASSEL_2 + MC_2 + TAIE; // SMCLK, contmode, interrupt
  _BIS_SR(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}

// Timer_A3 Interrupt Vector (TAIV) handler
#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A(void) {
  switch (TAIV) {
  case 2:
    break; // CCR1 not used
  case 4:
    break; // CCR2 not used
  case 10:
    P1OUT ^= 0x01; // overflow
    break;
  }
}