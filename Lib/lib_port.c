
#include "lib_port.h"
#include "main.h"
#include "print.h"
#include "key.h"
#include "terminal.h"
#include "led.h"

StreamBufferHandle_t sUartStreamBuff = NULL; //串口接收信号量
QueueHandle_t sLedQueue = NULL;

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
 * Function	: 按键中断处理，使用邮箱通知对应的任务
 * Input	: TaskHandle_t keyTask 任务指针
			  uint8_t id 按键ID
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
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
	}
}

/*
 * ============================================================================
 * Function	: 按键任务函数
 * Input	: void *parameter 参数指针
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
		//进入函数的时候不清除任务bit
		//退出函数的时候清除所有的bit
		//保存任务通知值
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
 * Function	: 调试串口中断处理，使用strembuff进行信息交互
 * Input	: uint8_t input 串口接收到的数据
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void terminal_irq_handle(uint8_t input)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (sUartStreamBuff != NULL)
	{
		xStreamBufferSendFromISR(sUartStreamBuff, &input, 1, &xHigherPriorityTaskWoken); //向队列中发送数据
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
	}
}

/*
 * ============================================================================
 * Function	: 调试串口任务函数
 * Input	: void *parameter 参数指针
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
 * Function	: 设置IO口的输出
 * Input	: uint8_t id io输出类ID
			  uint8_t state 1 打开， 0 关闭
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
 * Function	: LED任务函数
 * Input	: void *parameter 参数指针
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
 * Function	: 将设置LED灯控的信息写入消息队列
 * Input	: uint8_t id 灯ID
			  uint16_t onTime 开时间
			  uint16_t offTime 关时间
			  uint8_t repeat 重复次数
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