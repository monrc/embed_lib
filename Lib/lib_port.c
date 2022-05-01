
#include "lib_port.h"
#include "main.h"


/*
 * ============================================================================
 * Function	: 将数据写入串口发送寄存器
 * Input	: uint8_t byte
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void write_uart_tx_reg(uint8_t byte)
{
	USART2->DR = byte;
}

/*
 * ============================================================================
 * Function	: 使能串口发送空闲中断
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void enable_uart_tx_irq(void)
{
	SET_BIT(USART2->CR1, USART_CR1_TXEIE);
}

/*
 * ============================================================================
 * Function	: 禁用串口发送空闲中断
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void disable_uart_tx_irq(void)
{
	CLEAR_BIT(USART2->CR1, USART_CR1_TXEIE);
}

/*
 * ============================================================================
 * Function	: 获取串口的发送状态
 * Input	: None
 * Output	: None
 * Return	: true 串口忙	false 串口空闲
 * ============================================================================
 */
bool get_uart_tx_busy(void)
{
	if (READ_BIT(USART2->SR, UART_FLAG_TXE) == UART_FLAG_TXE)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
 * ============================================================================
 * Function	: 获取按键是否按下了
 * Input	: uint8_t id 按键序号
 * Output	: None
 * Return	: 0 未按下	1 按下了
 * ============================================================================
 */
uint8_t get_key_pressed(uint8_t id)
{
	uint8_t result = 0;

	switch (id)
	{
		case 0:
			result = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET ? 1 : 0;
			break;
		case 1:
			result = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET ? 1 : 0;
			break;
		case 2:
			result = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET ? 1 : 0;
			break;
		case 3:
			result = HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin);
		default:
			break;
	}

	return result;
}

/*
 * ============================================================================
 * Function	: 获取LED的状态
 * Input	: uint8_t id LED灯序号
			  uint8_t state 状态
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void set_led_state(uint8_t id, uint8_t state)
{
	GPIO_PinState pinState;

	pinState = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
	switch (id)
	{
		case 0:
			HAL_GPIO_WritePin(LED0_DS0_GPIO_Port, LED0_DS0_Pin, pinState);
			break;
		case 1:
			HAL_GPIO_WritePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin, pinState);
			break;
		default:
			break;
	}
}


void input_key_deal(uint8_t id, uint8_t keyType)
{
	
}