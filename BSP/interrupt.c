#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "print.h"
#include "app_layer.h"

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


void EXTI0_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);

	if (KeyTaskHandle != NULL)
	{
		xTaskNotifyFromISR(KeyTaskHandle, 0, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�Ļ�����һ�������л�
	}
}


void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);

}


void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);

}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}