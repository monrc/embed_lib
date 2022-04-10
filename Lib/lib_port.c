
#include "lib_port.h"
#include "main.h"


/*
 * ============================================================================
 * Function	: ������д�봮�ڷ��ͼĴ���
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
 * Function	: ʹ�ܴ��ڷ��Ϳ����ж�
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
 * Function	: ���ô��ڷ��Ϳ����ж�
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
 * Function	: ��ȡ���ڵķ���״̬
 * Input	: None
 * Output	: None
 * Return	: true ����æ	false ���ڿ���
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
