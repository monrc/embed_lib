
#include <string.h>

#include "main.h"
#include "bsp_spi.h"
#include "bsp_w25qxx.h"

#define SPI_CS_DISABLE	0
#define SPI_CS_ENABLE	1


#define enable_flash_cs()  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define disable_flash_cs() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)
#define enable_nrf_cs()	   HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_RESET)
#define disable_nrf_cs()   HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_SET)

static void set_spi_cs(uint8_t dev, uint8_t state);

QueueHandle_t gSpiQueue = NULL;
QueueHandle_t sSpiMutex = NULL;
TaskHandle_t sSpiTask = NULL;


/*
 * ============================================================================
 * Function	: SPI����FLASHоƬ
 * Input	: uint8_t *txBuff ���͵�����
			  uint8_t txSize �������ݳ���
			  uint8_t rxSize �������ݳ���
 * Output	: uint8_t *rxBuff ���յ�����
 * Return	: None
 * ============================================================================
 */
void spi_write_read_flash(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize)
{
	if (0 == txSize)
	{
		return;
	}

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

/*
 * ============================================================================
 * Function	: SPI����NRF24�ӿ�
 * Input	: uint8_t *txBuff ���͵�����
			  uint8_t txSize �������ݳ���
			  uint8_t rxSize �������ݳ���
 * Output	: uint8_t *rxBuff ���յ�����
 * Return	: None
 * ============================================================================
 */
void spi_write_read_nrf24(uint8_t *txBuff, uint8_t *rxBuff, uint8_t txSize, uint8_t rxSize)
{
	xSemaphoreTake(sSpiMutex, portMAX_DELAY);

	enable_nrf_cs();
	HAL_SPI_Transmit(&hspi2, txBuff, txSize, 100);
	if (rxSize)
	{
		HAL_SPI_Receive(&hspi2, rxBuff, rxSize, 100);
	}
	disable_nrf_cs();
	xSemaphoreGive(sSpiMutex);
}


/*
 * ============================================================================
 * Function	: SPI��ʼ������������SPI����
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void create_spi_task(void)
{
	gSpiQueue = xQueueCreate(3, sizeof(SpiMessage_t));
	sSpiMutex = xSemaphoreCreateMutex();

	xTaskCreate(spi_task, "spi", 512, NULL, 16, &sSpiTask);
}

/*
 * ============================================================================
 * Function	: SPI������
 * Input	: void *parameter ����ָ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void spi_task(void *parameter)
{
	uint32_t notifyValue = 0;
	SpiMessage_t message;

	while (1)
	{
		xQueueReceive(gSpiQueue, &message, portMAX_DELAY);

		xSemaphoreTake(sSpiMutex, portMAX_DELAY);
		set_spi_cs(message.type, SPI_CS_ENABLE);

		HAL_SPI_Transmit_DMA(&hspi2, message.txBuff, message.txSize);
		if (xTaskNotifyWait(0, ULONG_MAX, &notifyValue, message.txSize))
		{
			if (message.rxSize > 0)
			{
				HAL_SPI_Receive_DMA(&hspi2, message.rxBuff, message.rxSize);
				
				if (xTaskNotifyWait(0, ULONG_MAX, &notifyValue, message.txSize))
				{
					xTaskNotify(message.task, SPI2_RX_NOTIFY_BIT, eSetBits); //֪ͨ����������ݶ�ȡ
				}
			}
			else if (message.freeRam)	// FLASHд�����ݣ�����������Ҫ�ȴ�������ɣ��ҷ��ͻ�������̬���룬��Ҫ�ͷ�
			{
				vPortFree(message.txBuff);
			}
		}
		
		set_spi_cs(message.type, SPI_CS_DISABLE);
		xSemaphoreGive(sSpiMutex);
	}
}


/*
 * ============================================================================
 * Function	: SPI DMA�жϴ������͡���������жϴ���
 * Input	: uint32_t type �ж�����
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void spi2_irq_handle(uint32_t type)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (sSpiTask != NULL)
	{
		xTaskNotifyFromISR(sSpiTask, type, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�Ļ�����һ�������л�
	}
}

/*
 * ============================================================================
 * Function	: ����SPI CS �ܽ�״̬
 * Input	: uint8_t dev �豸
			  uint8_t state ״̬  SPI_CS_DISABLE SPI_CS_ENABLE
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void set_spi_cs(uint8_t dev, uint8_t state)
{
	if (dev == SPI_FLASH)
	{
		if (state == SPI_CS_DISABLE)
		{
			disable_flash_cs();
		}
		else
		{
			enable_flash_cs();
		}
	}
	else if (dev == SPI_NRF24)
	{
		if (state == SPI_CS_DISABLE)
		{
			disable_nrf_cs();
		}
		else
		{
			enable_nrf_cs();
		}
	}
}