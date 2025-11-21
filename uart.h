#ifndef __UART_H
#define __UART_H
#include "machine/_types.h"
#include <stdint.h>
// Module Prototypes
void uart_init_9600(void);
void uart_init_115200(void);
void uart_putc(char c);
void uart_print(const char *str);
void uart_print_int(int32_t num);
void uart_println(void);
void send_sample(float sample);
void send_sample_binary(float sample);

#endif
