
#include <string.h>

#include "main.h"
#include "bsp_24cxx.h"
#include "lib_port.h"

#define TXBUFFERSIZE 25
#define RXBUFFERSIZE 25
#define I2C_ADDRESS	 0x30F

#define AT24C02_Write 0xA0
#define AT24C02_Read  0xA1

uint8_t aTxBuffer[] = " ****I2C_TwoBoards communication based on DMA****  ****I2C_TwoBoards communication based on";

QueueHandle_t sEepromQueue = NULL;



#if 0
void bsp_24cxx_init(void)
{
	/*##-2- Start the transmission process #####################################*/
	/* While the I2C in reception process, user can transmit data through
	   "aTxBuffer" buffer */
	while (HAL_I2C_Master_Transmit_DMA(&hdma_i2c1_tx, (uint16_t)I2C_ADDRESS, (uint8_t *)aTxBuffer, TXBUFFERSIZE) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge its address)
		   Master restarts communication */
		if (HAL_I2C_GetError(&hdma_i2c1_tx) != HAL_I2C_ERROR_AF)
		{
			Error_Handler();
		}
	}

	/*##-3- Wait for the end of the transfer ###################################*/
	/*  Before starting a new communication transfer, you need to check the current
		state of the peripheral; if it’s busy you need to wait for the end of current
		transfer before starting a new one.
		For simplicity reasons, this example is just waiting till the end of the
		transfer, but application may perform other tasks while transfer operation
		is ongoing. */
	while (HAL_I2C_GetState(&hdma_i2c1_tx) != HAL_I2C_STATE_READY)
	{
	}


	/*##-4- Put I2C peripheral in reception process ###########################*/
	while (HAL_I2C_Master_Receive_DMA(&hdma_i2c1_tx, (uint16_t)I2C_ADDRESS, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge its address)
		   Master restarts communication */
		if (HAL_I2C_GetError(&hdma_i2c1_tx) != HAL_I2C_ERROR_AF)
		{
			Error_Handler();
		}
	}
}
#endif

void at_24cxx_write(void)
{
	uint8_t i;
	static uint8_t writeBuff[20] = {0};
	
	for (i = 0; i < 10; i++)
	{
		writeBuff[i] += 1;
	}

	if (HAL_I2C_Mem_Write(&hi2c1, AT24C02_Write, 0, I2C_MEMADD_SIZE_8BIT, writeBuff, 8, 1000) == HAL_OK)
	{
		debug("24cxx write ok\r\n");
	}
	
	if (HAL_I2C_Mem_Write_DMA(&hi2c1, AT24C02_Write, 0, I2C_MEMADD_SIZE_8BIT, writeBuff, 8) == HAL_OK)
	{
		debug("24cxx write ok\r\n");
	}
}


void at_24cxx_read(void)
{
	uint8_t readBuff[10] = {0};
	
	if (hi2c1.State != HAL_I2C_STATE_READY)
	{
		debug("state %02x\r\n", hi2c1.State);
		hi2c1.State = HAL_I2C_STATE_READY;
	}

	if (HAL_I2C_Mem_Read_DMA(&hi2c1, AT24C02_Read, 0, I2C_MEMADD_SIZE_8BIT, readBuff, 8) == HAL_OK)
	//if (HAL_I2C_Mem_Read(&hi2c1, AT24C02_Read, 0, I2C_MEMADD_SIZE_8BIT, readBuff, 10, 100) == HAL_OK)
	{
		debug("read ok\r\n");
	}

	HAL_Delay(500);
	print_array(LOG_PRINT, "rx", readBuff, 10);
}


/*
 * ============================================================================
 * Function	: EEPROM任务函数
 * Input	: void *parameter 参数指针
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void eeprom_task(void *parameter)
{
	EepromMessage_t msg;

	uint8_t size;
	uint8_t addr;
	uint8_t i;
	uint8_t sendBuff[10];

	led_init();

	sEepromQueue = xQueueCreate(5, sizeof(EepromMessage_t));

	while (1)
	{
		xQueueReceive(sEepromQueue, &msg, portMAX_DELAY);
		if (msg.type == AT24C02_Write)
		{
			addr = msg.addr;
			for (i = 0; i < msg.size; i += size)
			{
				size = msg.size - i;
				if (size > 8)
				{
					size = 8;
				}
				
				sendBuff[0] = addr;
				memcpy(&sendBuff[1], msg.buff + i, size);
				HAL_I2C_Master_Transmit_DMA(&hi2c1, AT24C02_Write, sendBuff, size + 1);
				addr += size;
				vTaskDelay(5);
			}
			vPortFree(msg.buff);
		}
		else if (msg.type == AT24C02_Read)
		{

			xTaskNotify(msg.task, 0x01, eSetBits);
		}
	}
}

void eeprom_write(uint8_t addr, uint8_t *data, uint8_t size)
{
	EepromMessage_t message;

	message.addr = addr;
	message.buff = (uint8_t *)pvPortMalloc(size);
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = AT24C02_Write;

	memcpy(message.buff, data, size);

	if (xQueueSend(sEepromQueue, &message, 20) != pdPASS)
	{
		vPortFree(message.buff);
	}
}


void eeprom_read(uint8_t addr, uint8_t *data, uint8_t size)
{
	EepromMessage_t message;

	message.addr = addr;
	message.buff = data;
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = AT24C02_Read;
	xQueueSend(sEepromQueue, &message, portMAX_DELAY);
	
	

}
