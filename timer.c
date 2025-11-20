#include <msp430f6736.h>
#include <stdint.h>

volatile uint16_t delay_remaining = 0;

void delay_ms(uint16_t ms) {
  delay_remaining = ms;

  // Stop timer & reset
  TA0CTL = TACLR;

  // ---- Configure Timer_A0 ----
  // SMCLK = 16.777216 MHz
  // We need 1 ms ticks:
  // 16,777,216 Hz / 1000 = 16777.216 → use CCR0 = 16777
  TA0CCR0 = 16777; // 1 ms interval
  TA0CCTL0 = CCIE; // enable interrupt

  TA0CTL = TASSEL__SMCLK | // Timer source = SMCLK
           MC__UP |        // Up mode
           TACLR;

  // Sleep in LPM0 (SMCLK stays on)
  __bis_SR_register(LPM0_bits | GIE);

  // Wakes here once all milliseconds are consumed
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
  if (delay_remaining > 0)
    delay_remaining--;

  if (delay_remaining == 0) {
    TA0CTL = MC__STOP;                    // Stop timer
    TA0CCTL0 &= ~CCIE;                    // Disable interrupt
    __bic_SR_register_on_exit(LPM0_bits); // Wake CPU
  }
}

// void delay_A1_ms(uint16_t ms)
// {
//     // Convert ms → ticks for ACLK
//     // ACLK = 32768 Hz XT1
//     // ticks = ms * 32.768
//     // Example: 1000 ms → 32768 ticks
//     //
//     // Use 32.768k ACLK because it runs in LPM3 and is stable/precise.
    
//     uint32_t ticks = ((uint32_t)ms * 32768) / 1000;

//     // Safety: clamp if needed
//     if (ticks > 65535)
//         ticks = 65535;

//     // Stop + reset Timer1_A
//     TA1CTL = TACLR;

//     TA1CCR0 = ticks;
//     TA1CCTL0 = CCIE;  // Enable CCR0 interrupt
//     TA1CTL = TASSEL__ACLK | MC__UP | TACLR; // Timer1 from ACLK, up mode

//     __bis_SR_register(LPM3_bits | GIE);   // Sleep until interrupt wakes us
    
//     // CPU continues here after ISR
// }

// // ISR wakes CPU after the delay
// #pragma vector=TIMER1_A0_VECTOR
// __interrupt void TIMER1_A0_ISR(void)
// {
//     TA1CTL = MC__STOP;      // Stop timer
//     TA1CCTL0 &= ~CCIE;      // Disable further interrupts

//     __bic_SR_register_on_exit(LPM3_bits);  // Exit LPM3 on ISR exit
// }
