#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdint.h>
// Declare the flag using 'extern'
// This tells all files that include this header that the variable
// key_event_ready is defined elsewhere.
extern volatile unsigned int key_event_ready;
extern volatile int32_t counts;

#endif
