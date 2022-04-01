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
	while ((USART2->SR & 0X40) == 0); //ѭ������,ֱ���������
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
* Function	: �Ӵ��ڻ�ȡһ���ַ�����
* Input		: uint32_t dwTimeOut ��ʱʱ��
* Output	: uint8_t *pChar �Ӵ����л�ȡ������
* Return	: true �ɹ�		false ʧ��
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
* Function	: ���ַ����ݷ���������
* Input		: uint8_t Data ���͵�����
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
* Function	: ���ַ�������������
* Input		: uint8_t *pStr Ҫ���͵��ַ���
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