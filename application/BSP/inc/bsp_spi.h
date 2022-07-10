#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <stdint.h>
#include "lib_port.h"

#define FLASH_WRITE		 1
#define FLASH_READ		 2

#define SPI_FLASH	1
#define SPI_NRF24	2

#define SPI2_TX_NOTIFY_BIT	0x01
#define SPI2_RX_NOTIFY_BIT	0x02


typedef struct
{
	uint8_t type;
	uint8_t freeRam;
	uint16_t waitTime;
	uint16_t txSize;
	uint16_t rxSize;
	uint8_t *txBuff;
	uint8_t *rxBuff;
	TaskHandle_t task;
} SpiMessage_t;

extern QueueHandle_t gSpiQueue;

void create_spi_task(void);
void spi_task(void *parameter);

void spi_write_read_flash(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize);
void spi_write_read_nrf24(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize);

void spi2_irq_handle(uint32_t type);

#endif
