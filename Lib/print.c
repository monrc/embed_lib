

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "print.h"
#include "circular_queue.h"

#include "lib_port.h"

// 配置项
#define MSG_LEN			 200	//最大单次输出长度
#define BUFF_SIZE		 200	//串口缓冲区大小
#define DEFAULT_MODULE	 0xFFFF //默认开启的模块
#define DEFAULT_LEVEL	 0x00	//默认打印等级
#define CRITICAL_PROTECT 2		// 0 无临界保护、 1 任务级临界保护、 2 中断级临界保护



//临界段保护方式
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

// 内部宏定义
uint8_t sBuff[QUEUE_SIZE(1, BUFF_SIZE)];
CircleQueue_t sUartQueue;
uint32_t sModuleMask = DEFAULT_MODULE | LOG_PRINT;
uint8_t sPrintLevel = DEFAULT_LEVEL;
uint8_t sUartIdle = true;

/*
 * ============================================================================
 * Function	: 调试输出初始化
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void debug_init(void)
{
	creat_queue(&sUartQueue, 1, sBuff);
	disable_uart_tx_irq();
}

/*
 * ============================================================================
 * Function	: 将输出的信息存入发送缓冲区
 * Input	: uint32_t module 模块
			  uint8_t level 打印等级
			  const char *format 打印参数列表
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print(uint32_t module, uint8_t level, const char *format, ...)
{
	uint8_t length;
	uint8_t firstByte;
	char buff[MSG_LEN];
	va_list ap;			  //声明字符指针 ap
	va_start(ap, format); //初始化 ap 变量

	if ((module & sModuleMask) && level > sPrintLevel)
	{
		length = vsnprintf(buff, MSG_LEN, format, ap); //使用参数列表发送格式化输出到字符串
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
		write_uart_tx_reg(firstByte);
		enable_uart_tx_irq();
	}
}

/*
 * ============================================================================
 * Function	: 将一组数据输出至缓冲区
 * Input	: uint8_t *buff 数组
			  uint8_t size 数组大小
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
		write_uart_tx_reg(firstByte);
		enable_uart_tx_irq();
	}
}

/*
 * ============================================================================
 * Function	: 打印一段数组数据
 * Input	: uint32_t module 模块
			  const char *name 数组名
			  uint8_t *array 要打印的数组
			  uint8_t size 数组大小
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
 * Function	: 打印错误
 * Input	: const char *file 文件名
			  uint32_t line 行数
			  const char *format 打印参数列表
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_error(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size = 0;
	char txBuff[MSG_LEN];
	va_list ap;			  //声明字符指针 ap
	va_start(ap, format); //初始化 ap 变量

	size = sprintf(txBuff, RED "Error \"%s\" line %u, ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //使用参数列表发送格式化输出到字符串

	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: 打印断言
 * Input	: const char *file 文件名
			  uint32_t line 行数
			  const char *format 打印参数列表
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_assert(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size;
	char txBuff[MSG_LEN];
	va_list ap; //声明字符指针 ap

	va_start(ap, format); //初始化 ap 变量
	size = sprintf(txBuff, RED "Assert \"%s\" line %u, ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //使用参数列表发送格式化输出到字符串
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: 打印警告
 * Input	: const char *file 文件名
			  uint32_t line 行数
			  const char *format 打印参数列表
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_warning(const char *file, uint32_t line, const char *format, ...)
{
	uint8_t size;
	char txBuff[MSG_LEN];
	va_list ap; //声明字符指针 ap

	va_start(ap, format); //初始化 ap 变量
	size = sprintf(txBuff, YELLOW "Warning \"%s\" line %u, ", file, line);
	size += vsnprintf(&txBuff[size], MSG_LEN - size, format, ap); //使用参数列表发送格式化输出到字符串
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: 绿色打印提示信息
 * Input	: uint32_t module 模块
			  uint8_t level 打印等级
			  const char *format 打印参数列表
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void print_prompt(uint32_t module, uint8_t level, const char *format, ...)
{
	uint8_t size = sizeof(GREEN) - 1;
	char txBuff[MSG_LEN];
	va_list ap; //声明字符指针 ap

	strcpy(txBuff, GREEN);

	va_start(ap, format); //初始化 ap 变量
	size += sprintf(&txBuff[size], format, ap);
	va_end(ap);

	strcpy(&txBuff[size], NONE);
	size += sizeof(NONE) - 1;

	uart_send_bytes((uint8_t *)txBuff, size);
}

/*
 * ============================================================================
 * Function	: 串口中断发送接口
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
		write_uart_tx_reg(byte);
	}
	else
	{
		disable_uart_tx_irq();
		sUartIdle = true;
	}
}

/*
 * ============================================================================
 * Function	: 串口发送一个字节并等待发送完成
 * Input	: uint8_t data 要发送的数据
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void uart2_output(uint8_t data)
{
	write_uart_tx_reg(data);

	while (get_uart_tx_busy()) //循环发送,直到发送完毕
	{
	}
}