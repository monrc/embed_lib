
#ifndef BSP_24CXX_H
#define BSP_24CXX_H

#include <stdint.h>
#include "lib_port.h"

typedef struct
{
	uint8_t type;
	uint8_t addr;
	uint8_t *buff;
	uint8_t size;
	TaskHandle_t task;
} EepromMessage_t;


#endif