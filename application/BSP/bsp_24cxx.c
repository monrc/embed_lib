
#include <string.h>

#include "main.h"
#include "bsp_24cxx.h"


#define READ_NOTIFIY_BIT	0x01
#define AT24C02_Write 0xA0
#define AT24C02_Read  0xA1

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
	uint8_t writeBuff[256] = {0};

	for (i = 0; i < 64; i++)
	{
		writeBuff[i] = i;
		writeBuff[i + 64] = i;
		writeBuff[i + 128] = i;
		writeBuff[i + 192] = i;
	}

	eeprom_write(0, writeBuff, 256, 20);


	// if (HAL_I2C_Mem_Write(&hi2c1, AT24C02_Write, 0, I2C_MEMADD_SIZE_8BIT, writeBuff, 8, 1000) == HAL_OK)
	// {
	// 	debug("24cxx write ok\r\n");
	// }
}


void at_24cxx_read(void)
{
	uint8_t readBuff[256] = {0};
	
	eeprom_read(0, readBuff, 256, 20);

	
	print_array(LOG_PRINT, "Bank1", &readBuff[0], 64);
	print_array(LOG_PRINT, "Bank2", &readBuff[64], 64);
	print_array(LOG_PRINT, "Bank3", &readBuff[128], 64);
	print_array(LOG_PRINT, "Bank4", &readBuff[192], 64);

	// if (hi2c1.State != HAL_I2C_STATE_READY)
	// {
	// 	debug("state %02x\r\n", hi2c1.State);
	// 	hi2c1.State = HAL_I2C_STATE_READY;
	// }

	// if (HAL_I2C_Mem_Read_DMA(&hi2c1, AT24C02_Read, 0, I2C_MEMADD_SIZE_8BIT, readBuff, 8) == HAL_OK)
	// //if (HAL_I2C_Mem_Read(&hi2c1, AT24C02_Read, 0, I2C_MEMADD_SIZE_8BIT, readBuff, 10, 100) == HAL_OK)
	// {
	// 	debug("read ok\r\n");
	// }

	// HAL_Delay(500);
	// print_array(LOG_PRINT, "rx", readBuff, 10);
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
	EepromMessage_t message;
	uint16_t size, i;
	uint8_t addr;
	uint8_t pageRemain;
	uint32_t notifyValue;
	uint8_t *buff;

	sEepromQueue = xQueueCreate(3, sizeof(EepromMessage_t));

	while (1)
	{
		xQueueReceive(sEepromQueue, &message, portMAX_DELAY);
		
		if (message.type == AT24C02_Write)	//写数据
		{
			addr = message.addr;
			buff = message.buff;
			
			for (i = 0; i < message.size; i += size)
			{
				size = message.size - i;	//本次需要写入的个数
				if (size > 8)
				{
					size = 8;
				}

				pageRemain = 8 - addr % 8;	//依据写入地址获取剩余的可以写入的个数
				if (pageRemain < 8 && pageRemain < size)
				{
					size = pageRemain;
				}

				if (HAL_I2C_Mem_Write_DMA(&hi2c1, AT24C02_Write, addr, I2C_MEMADD_SIZE_8BIT, buff, size) != HAL_OK)
				{
					debug_error("eeprom write fail\r\n");
				}

				addr += size;
				buff += size;
				vTaskDelay(5);
			}

			vPortFree(message.buff);
		}
		else if (message.type == AT24C02_Read)	//读数据
		{
			if (HAL_I2C_Mem_Read_DMA(&hi2c1, AT24C02_Read, message.addr, I2C_MEMADD_SIZE_8BIT, message.buff, message.size) != HAL_OK)
			{
				debug_error("eeprom read fail\r\n");
			}

			if (xTaskNotifyWait(0, ULONG_MAX, &notifyValue, 5))
			{
				xTaskNotify(message.task, READ_NOTIFIY_BIT, eSetBits);	//通知任务完成数据读取
			}
		}
	}
}

/*
 * ============================================================================
 * Function	: EEPROM，DMA读取完成中断处理，使用邮箱通知对应的任务
 * Input	: TaskHandle_t task 任务指针
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void eeprom_irq_handle(TaskHandle_t task)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (task != NULL)
	{
		xTaskNotifyFromISR(task, READ_NOTIFIY_BIT, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
	}
}

/*
 * ============================================================================
 * Function	: EEPROM写数据接口
 * Input	: uint8_t addr 地址
			  uint8_t *data 数据指针
			  uint8_t size 数据大小
			  uint32_t 写入等待的超时时间
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void eeprom_write(uint8_t addr, uint8_t *data, uint16_t size, uint32_t waitTime)
{
	EepromMessage_t message;

	message.addr = addr;
	message.buff = (uint8_t *)pvPortMalloc(size);
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = AT24C02_Write;

	memcpy(message.buff, data, size);

	if (xQueueSend(sEepromQueue, &message, waitTime) != pdPASS)
	{
		vPortFree(message.buff);
	}
}

/*
 * ============================================================================
 * Function	: EEPROM读取数据接口
 * Input	: uint8_t addr 地址
			  uint8_t size 读取的字节大小
			  uint32_t waitTime 读取等待的超时时间
 * Output	: uint8_t *data 数据指针
 * Return	: None
 * ============================================================================
 */
uint8_t eeprom_read(uint8_t addr, uint8_t *data, uint16_t size, uint32_t waitTime)
{
	EepromMessage_t message;
	uint32_t notifyValue;

	message.addr = addr;
	message.buff = data;
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = AT24C02_Read;
	xQueueSend(sEepromQueue, &message, portMAX_DELAY);
	
	if (xTaskNotifyWait(0, READ_NOTIFIY_BIT, &notifyValue, waitTime))
	{
		if (notifyValue & READ_NOTIFIY_BIT)
		{
			return true;
		}
		else
		{
			return false; 
		}
	}
	else
	{
		return false;
	}
}

