
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "print.h"
#include "circular_queue.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "stm32f1xx_hal.h"

// ������
#define MSG_LEN			 200	//��󵥴��������
#define BUFF_SIZE		 200	//���ڻ�������С
#define DEFAULT_MODULE	 0xFFFF //Ĭ�Ͽ�����ģ��
#define DEFAULT_LEVEL	 0x00	//Ĭ�ϴ�ӡ�ȼ�
#define CRITICAL_PROTECT 2		// 0 ���ٽ籣���� 1 �����ٽ籣���� 2 �жϼ��ٽ籣��

// MCU UART �Ĵ���
#define check_tx_busy()		 (READ_BIT(USART2->SR, UART_FLAG_TXE) != UART_FLAG_TXE)
#define enable_tx_irq()		 SET_BIT(USART2->CR1, USART_CR1_TXEIE)
#define disable_tx_irq()	 CLEAR_BIT(USART2->CR1, USART_CR1_TXEIE)
#define uart_send_data(byte) USART2->DR = byte

//�ٽ�α�����ʽ
#if (CRITICAL_PROTECT == 2)
#include "FreeRTOS.h"
#define enter_critical() uint32_t interrupt = portSET_INTERRUPT_MASK_FROM_ISR();
#define exit_critical()	 portCLEAR_INTERRUPT_MASK_FROM_ISR(interrupt)
#elif (CRITICAL_PROTECT == 1)
#include "FreeRTOS.h"
#define enter_critical() portENTER_CRITICAL()
#define exit_critical()	 portEXIT_CRITICAL()
#else
#define enter_critical()
#define exit_critical()
#endif

// �ڲ��궨��
#define UART_STATE_START 0x01

uint8_t sBuff[QUEUE_SIZE(1, BUFF_SIZE)];
CircleQueue_t sUartQueue;
uint32_t sModuleMask = DEFAULT_MODULE | LOG_PRINT;
uint8_t sPrintLevel = DEFAULT_LEVEL;
uint8_t sUartIdle = true;

/*
 * ============================================================================
 * Function	: ���������ʼ��
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void debug_init(void)
{
	creat_queue(&sUartQueue, 1, sBuff);
}

/*
 * ============================================================================
 * Function	: ���������Ϣ���뷢�ͻ�����
 * Input	: uint32_t module ģ��
			  uint8_t level ��ӡ�ȼ�
			  const char *format ��ӡ�����б�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print(uint32_t module, uint8_t level, const char *format, ...)
{
	uint8_t length;
	uint8_t firstByte;
	char buff[MSG_LEN];
	va_list ap;			  //�����ַ�ָ�� ap
	va_start(ap, format); //��ʼ�� ap ����

	if ((module & sModuleMask) && level > sPrintLevel)
	{
		length = vsnprintf(buff, MSG_LEN, format, ap); //ʹ�ò����б��͸�ʽ��������ַ���
	}

	while (1)
	{
		if (length <= get_remain_num(&sUartQueue))
		{
			en_queue_bytes(&sUartQueue, buff, length);
			break;
		}
		else
		{
			vTaskDelay(length / 13);
		}
	}
	va_end(ap);

	if (sUartIdle)
	{
		sUartIdle = false;
		de_queue(&sUartQueue, &firstByte);
		uart_send_data(firstByte);
		enable_tx_irq();
	}
}

/*
 * ============================================================================
 * Function	: ��һ�����������������
 * Input	: uint8_t *buff ����
			  uint8_t size �����С
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void uart_send_bytes(uint8_t *buff, uint8_t size)
{
	uint8_t firstByte;

	while (1)
	{
		if (size <= get_remain_num(&sUartQueue))
		{
			en_queue_bytes(&sUartQueue, buff, size);
			break;
		}
		else
		{
			vTaskDelay(size >> 3);
		}
	}

	if (sUartIdle)
	{
		sUartIdle = false;
		de_queue(&sUartQueue, &firstByte);
		uart_send_data(firstByte);
		enable_tx_irq();
	}
}

/*
 * ============================================================================
 * Function	: ��ӡһ����������
 * Input	: uint32_t module ģ��
			  const char *name ������
			  uint8_t *array Ҫ��ӡ������
			  uint8_t size �����С
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_array(uint32_t module, const char *name, uint8_t *array, uint8_t size)
{
	uint8_t i;
	char txBuff[MSG_LEN] = {'\0'};
	char *ptr;
	uint8_t len;

	if (module & sModuleMask)
	{
		strcpy(txBuff, name);
		len = strlen(txBuff);

		ptr = txBuff + len;
		for (i = 0; i < size; i++)
		{
			sprintf(ptr, " %02x", array[i]);
			ptr += 3;
		}
		len += size * 3;
		strcpy(ptr, "\r\n");

		uart_send_bytes((uint8_t *)txBuff, len + 2);
	}
}

/*
 * ============================================================================
 * Function	: ��ӡ����
 * Input	: const char *file �ļ���
			  uint32_t line ����
			  const char *format ��ӡ�����б�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_error(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size = 0;
	char txBuff[MSG_LEN];
	va_list ap;			  //�����ַ�ָ�� ap
	va_start(ap, format); //��ʼ�� ap ����

	size = sprintf(txBuff, RED "Error \"%s\" failed at line %u ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //ʹ�ò����б��͸�ʽ��������ַ���

	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: ��ӡ����
 * Input	: const char *file �ļ���
			  uint32_t line ����
			  const char *format ��ӡ�����б�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_assert(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size;
	char txBuff[MSG_LEN];
	va_list ap; //�����ַ�ָ�� ap

	va_start(ap, format); //��ʼ�� ap ����
	size = sprintf(txBuff, RED "Assert \"%s\" failed at line %u ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //ʹ�ò����б��͸�ʽ��������ַ���
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: ��ӡ����
 * Input	: const char *file �ļ���
			  uint32_t line ����
			  const char *format ��ӡ�����б�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_warning(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size;
	char txBuff[MSG_LEN];
	va_list ap; //�����ַ�ָ�� ap

	va_start(ap, format); //��ʼ�� ap ����
	size = sprintf(txBuff, YELLOW "Warning \"%s\" failed at line %lu ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //ʹ�ò����б��͸�ʽ��������ַ���
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: ��ɫ��ӡ��ʾ��Ϣ
 * Input	: uint32_t module ģ��
			  uint8_t level ��ӡ�ȼ�
			  const char *format ��ӡ�����б�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_prompt(uint32_t module, uint8_t level, const char *format, ...)
{
	uint8_t size = sizeof(GREEN) - 1;
	char txBuff[MSG_LEN];
	va_list ap; //�����ַ�ָ�� ap

	strcpy(txBuff, GREEN);

	va_start(ap, format); //��ʼ�� ap ����
	size += sprintf(&txBuff[size], format, ap);
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: �����жϷ��ͽӿ�
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void uart_send_irq(void)
{
	uint8_t byte;
	if (de_queue(&sUartQueue, &byte))
	{
		uart_send_data(byte);
	}
	else
	{
		disable_tx_irq();
		sUartIdle = true;
	}
}

/*
 * ============================================================================
 * Function	: ���ڷ���һ���ֽڲ��ȴ��������
 * Input	: uint8_t data Ҫ���͵�����
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void uart2_output(uint8_t data)
{
	uart_send_data(data);

	while (check_tx_busy()) //ѭ������,ֱ���������
	{
	}
}