// printf.c
#include <stdio.h>
#include <uart.h>

// Hex-only version: prints n in uppercase hex with optional width/pad
void uart_print_u32_hex(uint32_t n, int width, char pad) {
  // max 8 hex digits for 32-bit; +1 if you want safety
  char buf[8];
  size_t i = 0;

  // Special case zero
  if (n == 0) {
    if (width > 0) {
      while (--width > 0)
        uart_putc(pad);
    }
    uart_putc('0');
    return;
  }

  // Build digits in reverse order using nibbles
  while (n != 0) {
    uint8_t nibble = (uint8_t)(n & 0xF);           // low 4 bits
    buf[i++] = (nibble < 10)
                 ? (char)('0' + nibble)
                 : (char)('A' + (nibble - 10));    // uppercase hex
    n >>= 4;                                      // divide by 16 via shift
  }

  // Padding (if width > actual digits)
  if (width > i) {
    width -= i;
    while (width-- > 0)
      uart_putc(pad);
  }

  // Digits are reversed in buf[0..i-1], print backwards
  while (i > 0)
    uart_putc(buf[--i]);
}
