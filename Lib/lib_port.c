
#include "lib_port.h"
#include "main.h"
#include "print.h"
#include "key.h"


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

/*
 * ============================================================================
 * Function	: ��ȡ�����Ƿ�����
 * Input	: uint8_t id �������
 * Output	: None
 * Return	: 0 δ����	1 ������
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
 * Function	: �����жϴ���ʹ������֪ͨ��Ӧ������
 * Input	: TaskHandle_t keyTask ����ָ��
 			  uint8_t id ����ID
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void key_irq_handle(TaskHandle_t keyTask, uint8_t id)
{
	BaseType_t xHigherPriorityTaskWoken;

	if (keyTask != NULL)
	{
		xTaskNotifyFromISR(keyTask, id, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�Ļ�����һ�������л�
	}
}

/*
 * ============================================================================
 * Function	: ����������
 * Input	: void *parameter ����ָ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void key_task(void *parameter)
{
	uint32_t NotifyValue;
	uint32_t waitTime = 10;
	KeyTaskParamter_t *para;

	para = (KeyTaskParamter_t *)parameter;

	set_click_key(para->id, para->scanTime, para->timeOut, para->longPress);

	while (1)
	{
		//���뺯����ʱ���������bit
		//�˳�������ʱ��������е�bit
		//��������ֵ֪ͨ
		if (xTaskNotifyWait(0, ULONG_MAX, &NotifyValue, waitTime))
		{
			increase_count(NotifyValue);
		}

		click_key_deal(para->id, &waitTime);
		debug("id %u t %u\r\n", para->id, waitTime);
	}
}

/*
 * ============================================================================
 * Function	: ��ȡLED��״̬
 * Input	: uint8_t id LED�����
			  uint8_t state ״̬
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