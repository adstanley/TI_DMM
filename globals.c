// globals.c

#include <stdint.h>
#include "globals.h"

volatile unsigned int key_event_ready = 0;
volatile int32_t counts = 0;
