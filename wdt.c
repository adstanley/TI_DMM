#include <stdlib.h>
#include <msp430f6736.h>

inline void WDT_Enable(void) {
  WDTCTL = WDTPW + WDTSSEL_2 + WDTTMSEL +
           WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
  SFRIE1 |= WDTIE;
}

inline void WDT_Disable(void) {
  WDTCTL = WDTPW + WDTHOLD + WDTSSEL_2 + WDTTMSEL +
           WDTIS_7; // VLOCLK source, interval timer, 6.4mS per tick
  SFRIE1 &= ~WDTIE;
}
