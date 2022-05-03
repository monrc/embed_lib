#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "print.h"
#include "app_layer.h"
#include "lib_port.h"

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


void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);

	key_irq_handle(keyTask[3], 3);
}


void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
	key_irq_handle(keyTask[2], 2);
}


void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
	key_irq_handle(keyTask[1], 1);
}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
	key_irq_handle(keyTask[0], 0);
}


void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}