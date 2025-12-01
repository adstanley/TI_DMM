// uart.h
#ifndef __UART_H
#define __UART_H

#include "machine/_types.h"
#include <stdint.h>

// Module Prototypes
void uart_init_9600(void);
void uart_init_115200(void);
void uart_putc(char c);
void uart_print(const char *str);
void uart_print_integer(int32_t num);
void uart_println(void);
void uart_println_interrupt(void);
void send_sample(float sample);
void send_sample_interrupt(float sample);
void send_sample_binary(float sample);
void uart_print_interrupt(const char *s);
void uart_print(const char *s);
void uart_printf(const char *fmt, ...);
void uart_print_integer_interrupt(uint32_t val);
void uart_print_float(float value, uint8_t decimal_places);
void float_to_str(float value, char *buf, uint8_t decimal_places);
void uart_print_signed_integer_interrupt(int32_t val);
uint8_t uart_getc();

#endif
