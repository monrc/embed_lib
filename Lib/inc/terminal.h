#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define DEFAULT_AUTHORITY		0
#define USER_NUMBER				2		//�û�����
#define USER_NAME_TESTER		"tester"
#define	USER_KEY_TESETER		123
#define USER_NAME_ADMIN			"admin"
#define	USER_KEY_ADMIN			12345



#define RECV_BUFF_MAX_SIZE			99
#define RECV_BUFF_ARRAY_SIZE		(RECV_BUFF_MAX_SIZE + 1)
#define FUN_ARGUMENTS_MAX_SIZE		5


#define RECV_BUFF_OVERFLOW			32

#define	INPUT_KEY_UP			33
#define INPUT_KEY_DOWN			34
#define INPUT_KEY_RIGHT			35
#define INPUT_KEY_LEFT			36



#define CHAR_TO_SPECIAL(x)			((x) - 65 + 33)


//#pragma anon_unions

typedef struct
{
	void (*func)();
	const char *pName;
	const uint8_t paramterNum;
} Function_map_t;

typedef union
{
	struct
	{
		uint8_t state: 2;
		uint8_t authority: 2;
		uint8_t res: 4;
	};
	uint8_t val;
} Control_t;

typedef enum
{
	TERMINAL_IDEL,			//����״̬,�ȴ������һ���ַ�
	TERMINAL_BUSY,			//������,�ȴ��������
	TERMINAL_READY,			//�����˿��Ʒ����ȴ���������
} Terminal_state_t;

typedef struct
{
	void (*OutPutCallBack)(uint8_t data);		//��ӡ���ݻص�����
	char recvBuff[RECV_BUFF_ARRAY_SIZE];		//���ջ�����
	uint8_t recvLen;							//���ճ���
	uint8_t showLen;							//������Գ���
	uint8_t recvLast;							//���յ����ݻ������
	uint8_t ctrlType;							//�����ַ�����
	uint8_t userIndexTab[USER_NUMBER];			//�û���½��������������е�����
	uint8_t authority;							//����Ȩ�޵ȼ�����ͬ�û���ͬ��������
	uint8_t specialCharFlag;					//������Ʒ��ű�־
	volatile Control_t flag;					//����״̬����״̬��־
} Terminal_t;


void terminal_input_predeal(uint8_t data);
void terminal_handler(void);
void terminal_init(void (*pCallBack)(uint8_t));

#endif
