#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <stdint.h>
#include "lib_port.h"


#define SPI_FLASH	1
#define SPI_NRF24	2

typedef struct
{
	uint8_t type;
	uint16_t waitTime;
	uint16_t txSize;
	uint16_t rxSize;
	uint8_t *txBuff;
	uint8_t *rxBuff;
	TaskHandle_t task;
} SpiMessage_t;

#endif
