/*==============================================================================
  UART.h - UART module
==============================================================================*/

#ifndef __UART_H__
#define __UART_H__

#include <stdio.h>
#include <stdint.h>

// Initialize UART (call once from system_setup())
void uart_init(void);

// Optional: change baud rate on the fly (not needed for DMM)
void uart_set_baud(uint32_t baud);

// Print the current measurement over UART
// Call this whenever sampling_completed = 1
void uart_print_measurement(void);

#endif // __PROCESSING_H__
