
#ifndef BSP_24CXX_H
#define BSP_24CXX_H

#include <stdint.h>
#include "lib_port.h"

typedef struct
{
	uint8_t type;
	uint8_t addr;
	uint8_t *buff;
	uint16_t size;
	TaskHandle_t task;
} EepromMessage_t;

void create_eeprom_task(void);

void eeprom_task(void *parameter);

void eeprom_irq_handle(void);

void eeprom_write(uint8_t addr, uint8_t *data, uint16_t size, uint32_t waitTime);

uint8_t eeprom_read(uint8_t addr, uint8_t *data, uint16_t size, uint32_t waitTime);

#endif