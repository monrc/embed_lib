
#include <string.h>

#include "main.h"
#include "bsp_spi.h"
#include "bsp_w25qxx.h"

QueueHandle_t sSpiQueue = NULL;
QueueHandle_t sSpiMutex = NULL;
uint16_t sWaitTime = 10;

#define enable_flash_cs()	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define disable_flash_cs()	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)
#define enable_flash_cs()	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define disable_flash_cs()	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)


	uint16_t size, i;
	uint8_t addr;
	uint8_t pageRemain;
	uint32_t notifyValue;
	uint8_t *buff;


void spi_write_flash(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize)
{
	xSemaphoreTake(sSpiMutex, portMAX_DELAY);
	
	enable_flash_cs();
	HAL_SPI_Transmit(&hspi2, txBuff, txSize, 100);
	if (rxSize)
	{
		HAL_SPI_Receive(&hspi2, rxBuff, rxSize, 100);
	}
	disable_flash_cs();
	xSemaphoreGive(sSpiMutex);
}


void spi_write_nrf24(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize)
{

}




void spi2_task(void)
{
	SpiMessage_t message;
	sSpiQueue = xQueueCreate(3, sizeof(SpiMessage_t));
	sSpiMutex = xSemaphoreCreateMutex();

	while (1)
	{
		xQueueReceive(sSpiQueue, &message, portMAX_DELAY);

		if (message.type == SPI_FLASH)
		{
			
		}
		else if (message.type == SPI_NRF24)
		{

		}

		if (message.type == FLASH_WRITE) //写数据
		{
			addr = message.addr;
			buff = message.buff;

			for (i = 0; i < message.size; i += size)
			{
				size = message.size - i; //本次需要写入的个数
				if (size > 8)
				{
					size = 8;
				}

				pageRemain = 8 - addr % 8; //依据写入地址获取剩余的可以写入的个数
				if (pageRemain < 8 && pageRemain < size)
				{
					size = pageRemain;
				}

				// if (HAL_I2C_Mem_Write_DMA(&hi2c1, AT24C02_Write, addr, I2C_MEMADD_SIZE_8BIT, buff, size) != HAL_OK)
				// {
				// 	debug_error("eeprom write fail\r\n");
				// }

				addr += size;
				buff += size;
				vTaskDelay(5);
			}

			vPortFree(message.buff);
		}
		else if (message.type == FLASH_READ) //读数据
		{
			// if (HAL_I2C_Mem_Read_DMA(&hi2c1, AT24C02_Read, message.addr, I2C_MEMADD_SIZE_8BIT, message.buff,
			// 						 message.size) != HAL_OK)
			// {
			// 	debug_error("eeprom read fail\r\n");
			// }

			if (xTaskNotifyWait(0, ULONG_MAX, &notifyValue, 5))
			{
				xTaskNotify(message.task, READ_NOTIFIY_BIT, eSetBits); //通知任务完成数据读取
			}
		}
	}
}

void spi_write_read(W25QXX_t *info)
{
	if (info->txSize)
	{
		enable_spi_cs();

		HAL_SPI_Transmit(&hspi2, info->txBuff, info->txSize, 100);
		if (info->rxSize)
		{
			HAL_SPI_Receive(&hspi2, info->rxBuff, info->rxSize, 100);
		}

		disable_spi_cs();
	}
}


void 