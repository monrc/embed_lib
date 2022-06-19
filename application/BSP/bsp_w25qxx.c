

#include <string.h>
#include "main.h"
#include "bsp_w25qxx.h"

#define FLASH_WRITE		 1
#define FLASH_READ		 2
#define OLED_WRITE		 3
#define OLED_READ		 4
#define READ_NOTIFIY_BIT 0x40000000

#define WRITE_ENABLE		   0x06
#define SR_WRITE_ENABLE		   0x50
#define WRITE_DISABLE		   0x04
#define READ_STATUS1		   0x05
#define READ_STATUS2		   0x35
#define WRITE_STATUS		   0x01
#define PAGE_PROGRAM		   0x02
#define SECTOR_ERASE		   0x20
#define BLOCK_ERASE			   0x52
#define BLOCK_ERASE_64K		   0xD8
#define CHIP_ERASE			   0xC7
#define ERASE_PROGRAM_SUSPEND  0x75
#define ERASE_PROGRAM_RESUME   0x7A
#define POWER_DOWN			   0xB9
#define READ_DATA			   0x03
#define FAST_READ			   0x0B
#define RELEASE_POWERDOWN_ID   0xAB
#define MANUFACTURER_DEVICE_ID 0x90
#define JEDEC_ID			   0x9F
#define READ_UNIQUE_ID		   0x4B
#define READ_SFDP_REG		   0x5A
#define ERASE_SECURITY_REG	   0x44
#define PROGRAM_SECURITY_REG   0x42
#define READ_SECURITY_REG	   0x48
#define ENABLE_QPI			   0x38
#define ENABLE_RESET		   0x66
#define RESET				   0x99

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18

#define security1_addr(addr) (((addr) & 0xff) | 0x1000)
#define security2_addr(addr) (((addr) & 0xff) | 0x2000)
#define security3_addr(addr) (((addr) & 0xff) | 0x3000)
#define enable_spi_cs()		 HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define disable_spi_cs()	 HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)

QueueHandle_t sFlashQueue = NULL;
QueueHandle_t sSpiMutex = NULL;

uint16_t sWaitTime = 10;

void bsp_w25qxx_init(void)
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
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

/*
	等待时间 77
*/

/*
 * ============================================================================
 * Function	: 依据中断信号，增加中断计数值
 * Input	: uint8_t id 按键ID
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void w25qxx_single_cmd(uint8_t cmd)
{
	W25QXX_t info;

	switch (cmd)
	{
		case WRITE_ENABLE:
		case SR_WRITE_ENABLE:
		case WRITE_DISABLE:
		case CHIP_ERASE:
		case ERASE_PROGRAM_SUSPEND:
		case ERASE_PROGRAM_RESUME:
		case POWER_DOWN:
		case ENABLE_QPI:
		case ENABLE_RESET:
		case RESET:
			info.txSize = 1;
			info.cmd = cmd;
			info.rxSize = 0;
			break;
		default:
			info.txSize = 0;
			debug_warning("un support cmd\r\n");
			break;
	}

	spi_write_read(&info);
}

/*
 * ============================================================================
 * Function	: 读取状态寄存器
 * Input	: uint8_t cmd 指令 READ_STATUS1 READ_STATUS2
 * Output	: None
 * Return	: None
 * ============================================================================
 */
uint8_t w25qxx_read_status(uint8_t cmd)
{
	W25QXX_t info;

	info.cmd = cmd;
	info.txSize = 1;
	info.rxSize = 1;

	spi_write_read(&info);

	return info.rxStatus;
}

/*
 * ============================================================================
 * Function	: 写状态寄存器
 * Input	: uint8_t status1 状态寄存器1
			  uint8_t status2 状态寄存器2
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void w25qxx_write_status(uint8_t status1, uint8_t status2)
{
	W25QXX_t info;

	info.cmd = WRITE_STATUS;
	info.txSize = 3;
	info.rxSize = 0;
	info.txS1.value = status1;
	info.txS2.value = status2;

	spi_write_read(&info);

	vTaskDelay(15);
}

/*
 * ============================================================================
 * Function	: 写入数据至flash
 * Input	: uint8_t cmd 写入指令 PAGE_PROGRAM PROGRAM_SECURITY_REG
			  uint32_t addr 数据地址
			  uint8_t *buff 数据指针
			  uint8_t size 数据长度
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void w25qxx_page_program(uint8_t cmd, uint32_t addr, uint8_t *buff, uint8_t size)
{
	W25QXX_t info;

	info.cmd = cmd;
	info.txSize = 4;
	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	enable_spi_cs();
	HAL_SPI_Transmit_DMA(&hspi2, info.txBuff, info.txSize);
	HAL_SPI_Transmit_DMA(&hspi2, buff, size);
	disable_spi_cs();
}

/*
 * ============================================================================
 * Function	: 擦除指令
 * Input	: uint8_t cmd 擦除指令 SECTOR_ERASE BLOCK_ERASE BLOCK_ERASE_64K ERASE_SECURITY_REG
			  uint32_t addr 擦除地址
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void w25qxx_erase(uint8_t cmd, uint32_t addr)
{
	W25QXX_t info;

	info.cmd = cmd;
	info.txSize = 4;
	info.rxSize = 0;
	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	spi_write_read(&info);
}

/*
 * ============================================================================
 * Function	: 读取数据
 * Input	: uint8_t cmd 读取指令 READ_DATA FAST_READ READ_SECURITY_REG
			  uint32_t addr 读取数据的地址
			  uint8_t size 需要读取的数据大小
 * Output	: uint8_t *buff 读取的数据指针
 * Return	: None
 * ============================================================================
 */
void w25qxx_read_data(uint8_t cmd, uint32_t addr, uint8_t *buff, uint8_t size)
{
	W25QXX_t info;

	switch (cmd)
	{
		case FAST_READ:
		case READ_SECURITY_REG:
			info.cmd = cmd;
			info.txSize = 5;
			break;
		case READ_DATA:
			info.cmd = READ_DATA;
			info.txSize = 4;
			break;
		default:
			debug_warning("un support cmd\r\n");
			return;
	}

	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	enable_spi_cs();
	HAL_SPI_Transmit_DMA(&hspi2, info.txBuff, info.txSize);
	HAL_SPI_Receive_DMA(&hspi2, buff, size);
	disable_spi_cs();
}

uint8_t w25qxx_release_powerdown(void)
{
	W25QXX_t info;

	info.cmd = RELEASE_POWERDOWN_ID;
	info.txSize = 4;
	info.rxSize = 1;

	spi_write_read(&info);

	return info.devID;
}

void w25qxx_read_device_info(uint8_t *manufacturer, uint8_t *deviceID)
{
	W25QXX_t info;

	info.cmd = MANUFACTURER_DEVICE_ID;
	info.txSize = 4;
	info.rxSize = 2;
	info.res = 0;

	spi_write_read(&info);

	*manufacturer = info.manufacturer;
	*deviceID = info.deviceID;
}

void w25qxx_read_jedec_id(uint8_t *manufacturer, uint8_t *memoryType, uint8_t *capacity)
{
	W25QXX_t info;

	info.cmd = JEDEC_ID;
	info.txSize = 1;
	info.rxSize = 3;

	spi_write_read(&info);
	
	*manufacturer = info.manufact;
	*memoryType = info.memoryType;
	*capacity = info.capacity;
}

void w25qxx_read_uid(uint8_t *id)
{
	W25QXX_t info;

	info.cmd = READ_UNIQUE_ID;
	info.txSize = 5;
	info.rxSize = 8;
	spi_write_read(&info);
	memcpy(id, info.UID, 8);
}

uint8_t w25qxx_read_sfdp(uint8_t addr)
{
	W25QXX_t info;

	info.cmd = READ_SFDP_REG;
	info.txSize = 5;
	info.rxSize = 8;
	info.res5A[0] = 0;
	info.res5A[1] = 0;
	info.addr = addr;

	spi_write_read(&info);
	return info.SFDP;
}

void w25qxx_program_security_reg(uint32_t addr, uint16_t reg)
{
	W25QXX_t info;

	info.cmd = PROGRAM_SECURITY_REG;
	info.txSize = 6;
	info.rxSize = 0;

	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;
	info.data[0] = (uint8_t)(reg >> 8);
	info.data[1] = (uint8_t)reg;

	spi_write_read(&info);
}


void test(void)
{
	uint8_t cap;
	uint8_t type;
	uint8_t mf;

	w25qxx_read_jedec_id(&mf, &type, &cap);

	debug("mf %02x type %02x cap %02x\r\n", mf, type, cap);

	w25qxx_read_device_info(&mf, &type);

	debug("mf %02x type %02x\r\n", mf, type);
}


void flash_write_cmd(uint8_t cmd)
{
}

void w25qxx_write(void)
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

	//flash_write(0, writeBuff, 256, 20);

	// if (HAL_I2C_Mem_Write(&hi2c1, AT24C02_Write, 0, I2C_MEMADD_SIZE_8BIT, writeBuff, 8, 1000) == HAL_OK)
	// {
	// 	debug("24cxx write ok\r\n");
	// }
}

void w25qxx_read(void)
{
	uint8_t readBuff[256] = {0};

	//flash_read(0, readBuff, 256, 20);

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
void flash_task(void *parameter)
{
	FlashMessage_t message;
	uint16_t size, i;
	uint8_t addr;
	uint8_t pageRemain;
	uint32_t notifyValue;
	uint8_t *buff;

	sFlashQueue = xQueueCreate(3, sizeof(FlashMessage_t));
	
	sSpiMutex = xSemaphoreCreateMutex();

	while (1)
	{
		xQueueReceive(sFlashQueue, &message, portMAX_DELAY);

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

/*
 * ============================================================================
 * Function	: EEPROM，DMA读取完成中断处理，使用邮箱通知对应的任务
 * Input	: TaskHandle_t task 任务指针
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void eeprom_irq_handle1(TaskHandle_t task)
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
void flash_write(uint32_t addr, uint8_t *data, uint32_t size, uint32_t waitTime)
{
	FlashMessage_t message;

	message.addr = addr;
	message.buff = (uint8_t *)pvPortMalloc(size);
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = FLASH_WRITE;

	memcpy(message.buff, data, size);

	if (xQueueSend(sFlashQueue, &message, waitTime) != pdPASS)
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
uint8_t flash_read(uint8_t addr, uint8_t *data, uint16_t size, uint32_t waitTime)
{
	FlashMessage_t message;
	uint32_t notifyValue;

	message.addr = addr;
	message.buff = data;
	message.size = size;
	message.task = xTaskGetCurrentTaskHandle();
	message.type = FLASH_READ;
	xQueueSend(sFlashQueue, &message, portMAX_DELAY);

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


