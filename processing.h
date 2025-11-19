/****************************************************************
 * 	Texas Instruments, Kilby Labs
 *
 * 	Project: 		Fluxgate Demo
 *
 *  Author: 		Russ Rosenquist
 *  Date  :			January, 2014
 *
 *  **** TI Confidential - NDA Restrictions  ******
 *****************************************************************/

// processing.h
#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include <stdint.h>

// Variables used by other modules
uint8_t sampling_completed;     // Background.c
int64_t sampled_value;          // Background.c
uint8_t measurement_mode;       // Background.c
uint8_t Events;                 // Background.c
uint8_t key_debounce;           // Background.c
uint32_t active_voltage_offset; // Background.c
uint32_t active_current_offset; // Background.c

// Module Prototypes
void Main(void);
void set_voltage_range_and_gain_and_offset(enum i);
void set_current_range_and_gain_and_offset(CURRENT_RANGE i);
void set_power_range_and_gain_and_offset(POWER_RANGE i);
void off(void);
void update_display2(void);

#endif // __PROCESSING_H__
