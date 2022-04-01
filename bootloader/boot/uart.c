#include "uart.h"
#include "mcu_init.h"


#ifdef DEBUG_ON
__asm(".global __use_no_semihosting");

extern int  sendchar(int ch);  /* in Serial.c */
extern int  getkey(void);      /* in Serial.c */
extern long timeval;           /* in Time.c   */

FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	return (sendchar(ch));
}

int fgetc(FILE *f) {
	return (sendchar(getkey()));
}

int ferror(FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}

void _ttywrch(int ch)
{
	sendchar(ch);
}

void _sys_exit(int return_code)
{
	while (1);    /* endless loop */
}

int sendchar(int ch)
{
	while ((USART2->SR & 0X40) == 0); //循环发送,直到发送完毕
	USART2->DR = (uint8_t) ch;
	return ch;
}

int getkey(void)
{
	return 1;
}
#endif

/*********************************************************
* Name		: get_char
* Function	: 从串口获取一个字符数据
* Input		: uint32_t dwTimeOut 超时时间
* Output	: uint8_t *pChar 从串口中获取的数据
* Return	: true 成功		false 失败
*********************************************************/
bool get_char(uint8_t *pChar, uint32_t dwTimeOut)
{
	uint32_t ticks = HAL_GetTick();
	while(HAL_GetTick() - ticks < dwTimeOut)
	{
		if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
		{
			*pChar = (uint8_t)USART1->DR;
			return true;
		}
	}
	return false;
}

/*********************************************************
* Name		: put_char
* Function	: 将字符数据发送至串口
* Input		: uint8_t Data 发送的数据
* Output	: None
* Return	: None
*********************************************************/
void put_char(uint8_t Data)
{	
	USART1->DR = Data;
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET)
    {
		
    }
}

/*********************************************************
* Name		: put_string
* Function	: 将字符串发送至串口
* Input		: uint8_t *pStr 要发送的字符串
* Output	: None
* Return	: None
*********************************************************/
void put_string(char *pStr)
{
	while('\0' != *pStr)
	{
		put_char(*pStr++);
	}
}