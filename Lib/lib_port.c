
#include "lib_port.h"
#include "main.h"
#include "print.h"
#include "key.h"
#include "terminal.h"
#include "led.h"

StreamBufferHandle_t sUartStreamBuff = NULL; //���ڽ����ź���
QueueHandle_t sLedQueue = NULL;

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
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

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
	uint32_t notifyValue;
	uint32_t waitTime = 10;
	KeyTaskParamter_t *para;

	para = (KeyTaskParamter_t *)parameter;

	set_click_key(para->id, para->scanTime, para->timeOut, para->longPress);

	while (1)
	{
		//���뺯����ʱ���������bit
		//�˳�������ʱ��������е�bit
		//��������ֵ֪ͨ
		if (xTaskNotifyWait(0, ULONG_MAX, &notifyValue, waitTime))
		{
			increase_count(notifyValue);
		}

		click_key_deal(para->id, &waitTime);
		debug("id %u t %u\r\n", para->id, waitTime);
	}
}

/*
 * ============================================================================
 * Function	: ���Դ����жϴ���ʹ��strembuff������Ϣ����
 * Input	: uint8_t input ���ڽ��յ�������
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void terminal_irq_handle(uint8_t input)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (sUartStreamBuff != NULL)
	{
		xStreamBufferSendFromISR(sUartStreamBuff, &input, 1, &xHigherPriorityTaskWoken); //������з�������
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�Ļ�����һ�������л�
	}
}

/*
 * ============================================================================
 * Function	: ���Դ���������
 * Input	: void *parameter ����ָ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void terminal_task(void *parameter)
{
	uint8_t rxData;

	terminal_init(uart_send_byte);

	sUartStreamBuff = xStreamBufferCreate(10, 1);

	while (1)
	{
		if (xStreamBufferReceive(sUartStreamBuff, &rxData, 1, portMAX_DELAY))
		{
			terminal_input_predeal(rxData);
		}

		terminal_handler();
	}
}

/*
 * ============================================================================
 * Function	: ����IO�ڵ����
 * Input	: uint8_t id io�����ID
			  uint8_t state 1 �򿪣� 0 �ر�
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void set_io_output(uint8_t id, uint8_t state)
{
	GPIO_PinState reverse;

	reverse = state ? GPIO_PIN_RESET : GPIO_PIN_SET;

	switch (id)
	{
		case 0:
			HAL_GPIO_WritePin(LED0_DS0_GPIO_Port, LED0_DS0_Pin, reverse);
			break;
		case 1:
			HAL_GPIO_WritePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin, reverse);
			break;
		case 2:
			HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, state);
			break;
		default:
			break;
	}
}

/*
 * ============================================================================
 * Function	: LED������
 * Input	: void *parameter ����ָ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void led_task(void *parameter)
{
	uint32_t waitTime = portMAX_DELAY;
	LedMessage_t msg;
	
	led_init();

	sLedQueue = xQueueCreate(5, sizeof(LedMessage_t));

	while (1)
	{
		
		if (xQueueReceive(sLedQueue, &msg, waitTime))
		{
			set_output(msg.id, msg.onCount, msg.offCount, msg.repeat);
		}
		else
		{
			leds_output_deal();
		}
		
		waitTime = get_output_wait_time();
	}
}

/*
 * ============================================================================
 * Function	: ������LED�ƿص���Ϣд����Ϣ����
 * Input	: uint8_t id ��ID
			  uint16_t onTime ��ʱ��
			  uint16_t offTime ��ʱ��
			  uint8_t repeat �ظ�����
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void start_led_task(IODEVICE_t id, uint16_t onTime, uint16_t offTime, uint8_t repeat)
{
	LedMessage_t msg;

	if (sLedQueue != NULL)
	{
		msg.id = id;
		msg.onCount = TIME_TO_COUTN(onTime);
		msg.offCount = TIME_TO_COUTN(offTime);
		msg.repeat = repeat;
		xQueueSend(sLedQueue, &msg, 0);
	}
}



void input_key_deal(uint8_t id, uint8_t keyType)
{
}