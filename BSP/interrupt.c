#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "print.h"

void USART2_IRQHandler(void) //串口2中断服务程序
{
	uint8_t data;
	//BaseType_t xHigherPriorityTaskWoken;

	if (READ_BIT(USART2->SR, UART_FLAG_RXNE) == UART_FLAG_RXNE)
	{
		// data = USART2->DR & 0xff;
		// if (gUartQueueHandle != NULL)
		// {
		// 	xQueueSendFromISR(gUartQueueHandle, &data, &xHigherPriorityTaskWoken); //向队列中发送数据
		// 	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
		// }
	}

	if (READ_BIT(USART2->SR, UART_FLAG_TXE) == UART_FLAG_TXE)
	{
		uart_send_irq();
	}
}