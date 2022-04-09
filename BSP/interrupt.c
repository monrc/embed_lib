#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "print.h"

void USART2_IRQHandler(void) //����2�жϷ������
{
	uint8_t data;
	//BaseType_t xHigherPriorityTaskWoken;

	if (READ_BIT(USART2->SR, UART_FLAG_RXNE) == UART_FLAG_RXNE)
	{
		// data = USART2->DR & 0xff;
		// if (gUartQueueHandle != NULL)
		// {
		// 	xQueueSendFromISR(gUartQueueHandle, &data, &xHigherPriorityTaskWoken); //������з�������
		// 	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�Ļ�����һ�������л�
		// }
	}

	if (READ_BIT(USART2->SR, UART_FLAG_TXE) == UART_FLAG_TXE)
	{
		uart_send_irq();
	}
}