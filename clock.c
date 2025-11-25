#include <msp430f6736.h>
#include "hal_pmm.h"
#include "timer.h"

void Clock_Init_16MHz(void) {
  // Assume WDT was stopped in main() or startup code.

  // Required for 16+ MHz
  SetVCore(PMMCOREV_3); 
  
  // --- 1. Route XT1 crystal pins to the UCS ---
  // P5.0 = XIN, P5.1 = XOUT for the low-frequency crystal.
  P5SEL |= BIT0 | BIT1;

  // --- 2. Basic oscillator enable/disable (XT1 on, XT2 off) ---
  UCSCTL6 |= XT2OFF;      // Make sure high-frequency oscillator XT2 is OFF (we're not using it).
  UCSCTL6 &= ~XT1OFF;     // Turn ON XT1 (low-frequency crystal oscillator).

  // --- 3. Configure XT1 load capacitance ---
  // XT1 has internal programmable load caps. XCAP_3 ≈ 12.5 pF total, which
  // matches your MC-306 crystal's 12.5 pF load spec (with board stray).
  UCSCTL6 = (UCSCTL6 & ~(XCAP0 | XCAP1)) | XCAP_3;

  // --- 4. XT1 mode configuration ---
  // Clear:
  //   - XT1BYPASS: use internal amplifier (0), not external clock on pin.
  //   - XTS:       use low-frequency mode (0), not high-frequency mode.
  UCSCTL6 &= ~(XT1BYPASS | XTS);

  // Set XT1 drive strength:
  //   - Clear drive bits, then set to XT1DRIVE_2 (medium drive) for startup.
  //     Higher drive helps the crystal start reliably.
  UCSCTL6 = (UCSCTL6 & ~XT1DRIVE_3) | XT1DRIVE_2;

  // --- 5. Wait for all oscillators to stabilize ---
  // OFIFG (Oscillator Fault Flag) in SFRIFG1 is set if *any* of:
  //   - DCOFFG  (DCO fault)
  //   - XT1LFOFFG (low-freq XT1 fault)
  //   - XT2OFFG (XT2 fault)
  // are set in UCSCTL7.
  //
  // The recommended sequence is:
  //   - Clear all the local fault flags in UCSCTL7.
  //   - Clear OFIFG.
  //   - Re-read OFIFG; if still set, repeat until it stays cleared.
  do {
    // Clear local fault flags for DCO, XT1, and XT2 in UCSCTL7.
    UCSCTL7 &= ~(DCOFFG | XT1LFOFFG | XT2OFFG);

    // Clear the global oscillator fault flag.
    SFRIFG1 &= ~OFIFG;
  } while (SFRIFG1 & OFIFG);   // Loop until OFIFG stays cleared.

  // XT1 is now stable. Drop XT1 drive to the lowest setting for power saving.
  UCSCTL6 = (UCSCTL6 & ~XT1DRIVE_3) | XT1DRIVE_0;

  // --- 6. Configure the FLL to generate ~16.777216 MHz DCO ---
  // SCG0 = System Clock Generator control bit 0.
  // Setting SCG0 disables the FLL control loop so we can safely
  // program UCSCTL0/1/2/3 without the FLL fighting us.
  __bis_SR_register(SCG0);  // Disable FLL control loop.

  // UCSCTL0: DCOx and MODx bits (fine DCO tuning, modulation).
  // Set to 0: start from lowest internal DCO tap / no modulation.
  UCSCTL0 = 0x0000;

  // UCSCTL1: DCORSEL selects DCO frequency range.
  // DCORSEL_5 → range for roughly 16–33 MHz operation.
  UCSCTL1 = DCORSEL_5;      // Allow ~16 MHz DCO.

  // UCSCTL2: FLLN (multiplier) and FLLD (loop divider).
  //
  // FLL output frequency formula:
  //   F_DCO = (FLLN + 1) * F_REF / FLLD
  //
  // Here we program:
  //   - FLLD__1 → FLLD = 1 (no divide).
  //   - FLLN = 511.
  //
  // With F_REF = 32.768 kHz (we'll select REFO at that rate),
  // we get:
  //   F_DCO = (511 + 1) * 32768 / 1
  //         = 512 * 32768
  //         = 16.777216 MHz
  UCSCTL2 = FLLD__1 | (511);

  // --- 7. Select the FLL reference clock ---
  // UCSCTL3 controls:
  //   - SELREF: FLL reference source.
  //   - FLLREFDIV: reference divider.
  //
  // You *could* use the external crystal XT1 as the FLL reference:
  //   UCSCTL3 = SELREF__XT1CLK | FLLREFDIV__1;
  //
  // Instead, you're using the internal REFO as the FLL reference:
  //   SELREF_2 → REFOCLK (typically 32.768 kHz internal reference).
  //
  // This means:
  //   - XT1 runs and is used for ACLK,
  //   - but DCO is locked to REFO instead of XT1.
  // This is often preferable on metering devices, since REFO
  // keeps the FLL alive even if XT1 fails or is switched off.
  // (We OR in SELREF_2 so we don't disturb FLLREFDIV.)
  // FLLREFDIV is still effectively /1 here (unchanged).
  // UCSCTL3 = SELREF__XT1CLK | FLLREFDIV__1;  // <-- alternative: FLL tied to XT1
  UCSCTL3 |= SELREF_2; // FLL reference clock = REFOCLK (internal ~32.768 kHz)

  // Re-enable FLL loop so it can start driving the DCO toward
  // the target frequency set by UCSCTL1/2 and the REFO reference.
  __bic_SR_register(SCG0); // Clear SCG0 → FLL control loop enabled again.

  // --- 8. Select clock sources and dividers for ACLK, SMCLK, MCLK ---
  // UCSCTL4 selects the clock *sources*:
  //   - SELA__XT1CLK  → ACLK = XT1 (external 32.768 kHz crystal).
  //   - SELS__DCOCLK  → SMCLK = DCO.
  //   - SELM__DCOCLK  → MCLK = DCO.
  UCSCTL4 = SELA__XT1CLK    // ACLK  = XT1 (external 32k)
            | SELS__DCOCLK  // SMCLK = DCO (FLL output)
            | SELM__DCOCLK; // MCLK  = DCO (FLL output)

  // UCSCTL5 sets *dividers*:
  //   - DIVA__1 → ACLK divider   = /1 (so ACLK ≈ 32.768 kHz).
  //   - DIVS__1 → SMCLK divider  = /1 (so SMCLK ≈ 16.777 MHz).
  //   - DIVM__1 → MCLK divider   = /1 (so MCLK ≈ 16.777 MHz).
  UCSCTL5 = DIVA__1    // ACLK divider  = /1
            | DIVS__1  // SMCLK divider = /1
            | DIVM__1; // MCLK divider  = /1

  // --- 9. Allow time for DCO to settle / FLL to converge ---
  // Even after re-enabling the FLL, it takes some time (many cycles)
  // for the DCO to "walk" to the target frequency.
  // We spin here for a while so that by the time we exit this function,
  // MCLK/SMCLK are very close to the final ~16.78 MHz.
  //
  // At 16 MHz, 500000 cycles ≈ 31.25 ms.
  // During ramp-up it might be slightly different, but this is a safe wait.
  __delay_cycles(500000); // ~30 ms at ~16 MHz
  // delay_ms(50);
  // delay_A1_ms(50);

}


void Clock_Init_25MHz(void)
{
    // 1) Raise Vcore for 25 MHz operation
    SetVCore(PMMCOREV_3); 

    // 2) Make sure REFO is available as FLL reference
    //    (It is on by default, but this is explicit)
    UCSCTL3 = SELREF__REFOCLK;      // FLL ref = REFO (32.768 kHz)

    // 3) Optional: route ACLK from XT1 if your 32.768 kHz crystal is up
    //    If XT1 is not stable yet, you can set ACLK = REFO instead.
    UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | SELA__XT1CLK;  // ACLK = XT1
    // (SELS/SELM will be set after FLL config)

    // 4) Configure DCO for ~25 MHz via FLL
    __bis_SR_register(SCG0);        // Disable FLL control loop

    UCSCTL0 = 0x0000;               // Lowest DCOx, MODx
    UCSCTL1 = DCORSEL_6;            // DCO range that supports up to ~50 MHz
                                    // (needed for 25 MHz operation)

    // Fdco = (N + 1) * Fref
    // Fref = 32768 Hz (REFO), we want Fdco ≈ 25 MHz:
    // (N + 1) = 25,000,000 / 32,768 ≈ 763  → N = 762
    UCSCTL2 = 762;                  // FLLN = 762, FLLD = /1 (default)

    // FLL uses REFO as reference (already set in UCSCTL3)
    // FLLREFDIV left at /1 (FLLREFDIV__1)

    __bic_SR_register(SCG0);        // Re-enable FLL

    // 5) Select clock sources and dividers
    UCSCTL4 = (UCSCTL4 & ~(SELA_7 | SELS_7 | SELM_7))
            | SELA__XT1CLK          // ACLK  = XT1 (32.768 kHz)
            | SELS__DCOCLK          // SMCLK = DCO (~25 MHz)
            | SELM__DCOCLK;         // MCLK  = DCO (~25 MHz)

    UCSCTL5 = DIVA__1               // ACLK /1
            | DIVS__1               // SMCLK /1
            | DIVM__1;              // MCLK /1

    // 6) Let DCO/FLL settle
    // Worst case: 32 * 32 * f_MCLK / f_FLLREF
    // 32 * 32 * 25 MHz / 32768 Hz ≈ 781,250 cycles
    __delay_cycles(781250u);

    // 7) Clear oscillator fault flags (XT1 + DCO)
    do {
        UCSCTL7 &= ~(XT1LFOFFG | DCOFFG);   // F6736 has XT1LFOFFG, no XT1HFOFFG
        SFRIFG1 &= ~OFIFG;                 // Clear global osc fault
    } while (SFRIFG1 & OFIFG);
}
