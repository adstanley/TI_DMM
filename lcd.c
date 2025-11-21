
#include <msp430f6736.h>
#include <stdint.h>
#include <stdlib.h>

// Breaks build because variables are defined in main, comment out for now
// Wont use it anyway

// void update_display() {

//   LCDM1 = 0;  // main line, digit 6 number
//   LCDM2 = 0;  // main line, digit 6 starburst + sign
//   LCDM3 = 0;  // main line, digit 5 number
//   LCDM4 = 0;  // main line, digit 5 starburst + decimal point
//   LCDM5 = 0;  // main line, digit 4 number
//   LCDM6 = 0;  // main line, digit 4 starburst + decimal point
//   LCDM7 = 0;  // main line, digit 3 number
//   LCDM8 = 0;  // main line, digit 3 starburst + decimal point
//   LCDM9 = 0;  // main line, digit 2 number
//   LCDM10 = 0; // main line, digit 2 starburst + decimal point

//   switch (measurement_mode) {
//   case DC_VOLTAGE:
//     if (calibrate == 0) {
//       LCDM14 = 0xC7;
//       LCDM16 = 0xC9;
//       LCDM17 = 0xCF;
//     } else {
//       if (power_mode_cal == 0) {
//         LCDM14 = 0xC7;
//         LCDM16 = 0x00;
//         LCDM17 = 0xC9;
//       } else {
//         LCDM14 = 0xC7;
//         LCDM16 = 0xEC;
//         LCDM17 = 0xC9;
//       }
//     }
//     break;
//   case AC_VOLTAGE:
//     LCDM14 = 0xC7;
//     LCDM16 = 0xC9;
//     LCDM17 = 0xEE;
//     break;
//   case DC_CURRENT:
//     if (calibrate == 0) {
//       LCDM14 = 0xEE;
//       LCDM16 = 0xC9;
//       LCDM17 = 0xCF;
//     } else {
//       if (power_mode_cal == 0) {
//         LCDM14 = 0xEE;
//         LCDM16 = 0x00;
//         LCDM17 = 0xC9;
//       } else {
//         LCDM14 = 0xEE;
//         LCDM16 = 0xEC;
//         LCDM17 = 0xC9;
//       }
//     }
//     break;
//   case AC_CURRENT:
//     LCDM14 = 0xEE;
//     LCDM16 = 0xC9;
//     LCDM17 = 0xEE;
//     break;
//   case POWER:
//     LCDM14 = 0xEC;
//     LCDM16 = 0;
//     LCDM17 = 0;
//     break;
//   default:
//     break;
//   }

//   switch (unit) {
//   case 0: // none
//     LCDM11 = 0x0;
//     LCDM12 = 0x0;
//     break;
//   case 1: // milli
//     LCDM11 = 0x44;
//     LCDM12 = 0xa0;
//     break;
//   case 2: // micro
//     LCDM11 = 0x47;
//     LCDM12 = 0x00;
//     break;
//   default:
//     break;
//   }

//   switch (decimal_position) {
//   case 2:
//     LCDM8 |= 0x0001;
//     break;
//   case 3:
//     LCDM6 |= 0x0001;
//     break;
//   case 4:
//     LCDM4 |= 0x0001;
//     break;
//   case 5:
//     LCDM2 |= 0x0001;
//     break;
//   default:
//     break;
//   }

//   display_value = (int32_t)(displayed_result_fp);
//   if (display_value < 0) {
//     LCDM2 |= 0x04;
//     display_value = ~display_value + 1;
//   }

//   LCDM9 = LCD_Char_Map[display_value % 10];

//   display_value /= 10;
//   LCDM7 = LCD_Char_Map[display_value % 10];

//   display_value /= 10;
//   LCDM5 = LCD_Char_Map[display_value % 10];

//   display_value /= 10;
//   LCDM3 = LCD_Char_Map[display_value % 10];

//   display_value /= 10;
//   LCDM1 = LCD_Char_Map[display_value % 10];
// }