

#include <string.h>
#include "main.h"
#include "bsp_w25qxx.h"
#include "bsp_spi.h"


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


uint16_t sWaitTime = 10;


static void wait_chip_idle(void);


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

	wait_chip_idle();

	switch (cmd)
	{
		case CHIP_ERASE:
		case WRITE_ENABLE:
		case SR_WRITE_ENABLE:
		case WRITE_DISABLE:
		case ERASE_PROGRAM_SUSPEND:
		case ERASE_PROGRAM_RESUME:
		case POWER_DOWN:
		case ENABLE_QPI:
		case ENABLE_RESET:
		case RESET:
			if (CHIP_ERASE == cmd)
			{
				sWaitTime = 20000;
			}
			info.txSize = 1;
			info.cmd = cmd;
			info.rxSize = 0;
			break;
		default:
			info.txSize = 0;
			debug_warning("un support cmd\r\n");
			break;
	}

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);
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

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);

	return info.rxStatus;
}

/*
 * ============================================================================
 * Function	: 等待芯片空闲
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void wait_chip_idle(void)
{
	W25QXXStatus1_t status;

	while (1)
	{
		status.value = w25qxx_read_status(READ_STATUS1);
		if (status.busy)
		{
			vTaskDelay(sWaitTime);
		}
		else
		{
			break;
		}
	}
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
	
	wait_chip_idle();
	sWaitTime = 15;

	info.cmd = WRITE_STATUS;
	info.txSize = 3;
	info.rxSize = 0;
	info.txS1.value = status1;
	info.txS2.value = status2;

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);
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
	
	wait_chip_idle();
	sWaitTime = cmd == SECTOR_ERASE ? 60 : 150;
	
	info.cmd = cmd;
	info.txSize = 4;
	info.rxSize = 0;
	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);
}

/*
 * ============================================================================
 * Function	: 退出断电模式
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
uint8_t w25qxx_release_powerdown(void)
{
	W25QXX_t info;

	wait_chip_idle();

	info.cmd = RELEASE_POWERDOWN_ID;
	info.txSize = 4;
	info.rxSize = 1;

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);

	return info.devID;
}

/*
 * ============================================================================
 * Function	: 读取设备信息
 * Input	: None
 * Output	: uint8_t *manufacturer	厂商
			  uint8_t *deviceID 设备型号
 * Return	: None
 * ============================================================================
 */
void w25qxx_read_device_info(uint8_t *manufacturer, uint8_t *deviceID)
{
	W25QXX_t info;
	
	wait_chip_idle();

	info.cmd = MANUFACTURER_DEVICE_ID;
	info.txSize = 4;
	info.rxSize = 2;
	info.res = 0;

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);

	*manufacturer = info.manufacturer;
	*deviceID = info.deviceID;
}

/*
 * ============================================================================
 * Function	: 读取设备信息
 * Input	: None
 * Output	: uint8_t *manufacturer	厂商
			  uint8_t *memoryType 存储类型
			  uint8_t *capacity 容量
 * Return	: None
 * ============================================================================
 */
void w25qxx_read_jedec_id(uint8_t *manufacturer, uint8_t *memoryType, uint8_t *capacity)
{
	W25QXX_t info;

	wait_chip_idle();

	info.cmd = JEDEC_ID;
	info.txSize = 1;
	info.rxSize = 3;

	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);
	
	*manufacturer = info.manufact;
	*memoryType = info.memoryType;
	*capacity = info.capacity;
}

/*
 * ============================================================================
 * Function	: 读取设备信息
 * Input	: None
 * Output	: uint8_t *id 读取独特的ID号
 * Return	: None
 * ============================================================================
 */
void w25qxx_read_uid(uint8_t *id)
{
	W25QXX_t info;

	wait_chip_idle();

	info.cmd = READ_UNIQUE_ID;
	info.txSize = 5;
	info.rxSize = 8;
	spi_write_read_flash(info.txBuff, info.rxBuff, info.txSize, info.rxSize);
	memcpy(id, info.UID, 8);
}

/*
 * ============================================================================
 * Function	: 写入数据至flash
 * Input	: uint8_t cmd 写入指令 PAGE_PROGRAM PROGRAM_SECURITY_REG
 			  uint8_t addr 地址
			  uint8_t *buff 数据指针
			  uint8_t size 数据大小
			  uint32_t 写入等待的超时时间
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void w25qxx_page_program(uint8_t cmd, uint32_t addr, uint8_t *buff, uint32_t size, uint32_t waitTime)
{
	SpiMessage_t message;
	W25QXX_t info;

	info.cmd = cmd;
	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	message.type = SPI_FLASH;
	message.freeRam = true;
	message.txSize = size + 4;
	message.rxSize = 0;
	message.txBuff = (uint8_t *)pvPortMalloc(size + 4);
	message.task = xTaskGetCurrentTaskHandle();
	
	memcpy(message.txBuff, info.txBuff, 4);
	memcpy(message.txBuff + 4, buff, size);

	wait_chip_idle();
	sWaitTime = 1;

	if (xQueueSend(gSpiQueue, &message, waitTime) != pdPASS)
	{
		vPortFree(message.txBuff);	//写超时，释放数据
	}
}

 /*
 * ============================================================================
 * Function	: 读取数据
 * Input	: uint8_t cmd 读取指令 READ_DATA FAST_READ READ_SECURITY_REG
			  uint8_t addr 地址
			  uint8_t size 读取的字节大小
			  uint32_t waitTime 读取等待的超时时间
 * Output	: uint8_t *buff 读取的数据指针
 * Return	: None
 * ============================================================================
 */
uint8_t w25qxx_read_data(uint8_t cmd, uint8_t addr, uint8_t *buff, uint16_t size, uint32_t waitTime)
{
	SpiMessage_t message;
	uint32_t notifyValue;
	W25QXX_t info;

	switch (cmd)
	{
		case FAST_READ:
		case READ_SECURITY_REG:
		case READ_SFDP_REG:
			info.cmd = cmd;
			info.txSize = 5;
			break;
		case READ_DATA:
			info.cmd = READ_DATA;
			info.txSize = 4;
			break;
		default:
			debug_warning("not support cmd\r\n");
			return false;
	}

	info.addrHH = (uint8_t)(addr >> 16);
	info.addrHL = (uint8_t)(addr >> 8);
	info.addrLL = (uint8_t)addr;

	message.type = SPI_FLASH;
	message.freeRam	= false;
	message.txSize = info.txSize;
	message.txBuff = info.txBuff;
	message.rxSize = size;
	message.rxBuff = buff;
	message.task = xTaskGetCurrentTaskHandle();
	
	wait_chip_idle();
	
	xQueueSend(gSpiQueue, &message, portMAX_DELAY);

	if (xTaskNotifyWait(0, SPI2_RX_NOTIFY_BIT, &notifyValue, waitTime))
	{
		if (notifyValue & SPI2_RX_NOTIFY_BIT)
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


void test1(void)
{
	uint8_t cap;
	uint8_t type;
	uint8_t mf;
	uint8_t uid[8];

	w25qxx_read_jedec_id(&mf, &type, &cap);

	debug("mf %02x type %02x cap %02x\r\n", mf, type, cap);

	w25qxx_read_device_info(&mf, &type);

	debug("mf %02x type %02x\r\n", mf, type);

	w25qxx_read_uid(uid);

	print_array(LOG_PRINT, "UID", uid, 8);

	debug("status %x %x\r\n", w25qxx_read_status(READ_STATUS1), w25qxx_read_status(READ_STATUS2));
	
	w25qxx_single_cmd(WRITE_ENABLE);
	
	w25qxx_erase(SECTOR_ERASE, 0);
	program_pag(0, uid, 8, 1000);

	fast_read(0, uid, 8, 1000);
	print_array(LOG_PRINT, "data", uid, 8);
}