/********************************************************
*file name: terminal.c
*function: �����û������ָ�ִ�У�ָ�����֧��5������
*ʹ��˵����
	1.�ⲿ�ӿ�
		terminal_init() 			�ն��ڴ��ʼ������
		terminal_input_predeal() 	���ڴ��ڽ����ж��У�ǰ̨����
		terminal_handler()			������ѭ��while(1)�У���̨����
	2.��������
		Tab 		��ʾ����ƥ�������Զ�ƥ��ָ��
		��			������ʷ����
		��			ִ����ʷ����
		Enter		�����������ʼִ��
		Backspace	ɾ��ǰһ���ַ�
	3.���ָ��
		�����������stFunTab�����У���������Ϊ��������ָ��������������
	4.����ʾ��
		ָ���� �ո� ����1 �ո� ����2 �ո� ����3 �س�
		eg��test 123 345 �س�
	5.��ֲ˵��
		���ⲿ�ӿڷ��ں��ʵ�λ��
		�ض���ʵ��printf��������
		ʵ�ִ��ڴ�ӡ�ַ��ĺ�������ģ����ʹ��serial1_put_char
		��Ӻ���ָ��ӳ���
********************************************************/

#include <string.h>
#include <stdio.h>
#include "terminal.h"


//�ڲ���������
static void searching_command(void);
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd);
static bool string_to_uint(char *pStr, uint8_t len, uint32_t *pValue);
static void execute_handled(uint8_t funIndex, uint32_t *pArg, uint8_t ArgNum);
static bool recv_semantic_analysis(void);
static void input_echo(void);
static void output_string(const char *pStr, uint8_t len);
static bool check_login_cmd(uint8_t index);

static void tester_login(uint32_t key);
static void admin_login(uint32_t key);
static void software_reset(void);
static void user_help(void);



//����ָ��ӳ����壬����Ϊ������������ӳ��������βθ���
//ָ����ڵײ��������Ŵ�����Ȩ�޵Ĵ�С��ֵԽ��Ȩ��Խ��
const Function_map_t stFunTab[] =
{
	{user_help, 			"?", 				0},
	{software_reset, 		"reboot", 			0},
	{tester_login, 			USER_NAME_TESTER,   1},
	// {timer_list_print, 		"timerprint", 		0},
	// {task_list_print,  		"taskprint", 		0},
	// {timer_test, 		 	"timertest", 		1},
	// {task_test, 			"tasktest", 		1},
	// {task_list_pop, 		"pop", 				0},
	{admin_login, 			USER_NAME_ADMIN, 	1},
};
#define STFUNTAB_SIZE		(sizeof(stFunTab) / sizeof(stFunTab[0]))

//�ڲ��ն�ģ��ṹ�嶨��
static Terminal_t sTerminal;


/*
* ============================================================================
* Name		: terminal_handler
* Function	: �ն���������������
* Input		: None
* Output	: None
* Return	: None
* Note		: �ú����ʺϷ��ں�̨�����е���
* ============================================================================
*/
void terminal_handler(void)
{
	const char backSpace[3] = {'\b', ' ', '\b'};

	//����ͬ���뻥�������
	if (TERMINAL_READY != sTerminal.flag.state)
	{
		input_echo();	//�������
		return;
	}

	switch (sTerminal.ctrlType)
	{
		case '\b':	//backspace ����
		{
			sTerminal.recvLen = sTerminal.recvLen ? sTerminal.recvLen - 1 : 0;
			output_string(backSpace, 3);
			break;
		}

		case '\t':	//Tab ����
		{
			searching_command();
			break;
		}

		case '\r':	//Enter ����
		{
			printf("\r\n");
			recv_semantic_analysis();
			break;
		}

		case INPUT_KEY_UP:	//������
		{
			if (sTerminal.recvLen == 0)
			{
				if (' ' == sTerminal.recvBuff[sTerminal.recvLast - 1])
				{
					sTerminal.recvLast--;
				}

				sTerminal.recvLen = sTerminal.recvLast;
				output_string(sTerminal.recvBuff, sTerminal.recvLen);
			}
			break;
		}

		case INPUT_KEY_DOWN: //������
		{
			if (sTerminal.recvLen == 0)
			{
				sTerminal.recvLen = sTerminal.recvLast;
				output_string(sTerminal.recvBuff, sTerminal.recvLen);

				printf("\r\n");
				recv_semantic_analysis();
			}
			break;
		}

		case RECV_BUFF_OVERFLOW:	//���������
		{
			printf("uart recv over flow\r\n");
			sTerminal.recvLast = sTerminal.recvLen;
			sTerminal.recvLen = 0;
			break;
		}
		default :
			break;
	}
	sTerminal.flag.state = TERMINAL_IDEL;
}

/*
* ============================================================================
* Name		: terminal_init
* Function	: �ն�ģ�������ʼ��
* Input		: None
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
void terminal_init(void (*pCallBack)(uint8_t))
{
	uint8_t i;
	memset(&sTerminal, 0, sizeof(Terminal_t));
	sTerminal.OutPutCallBack = pCallBack; //���ڴ�ӡ����
	sTerminal.flag.state = TERMINAL_IDEL;
	sTerminal.authority = DEFAULT_AUTHORITY;

	//�����б���
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (NULL == stFunTab[i].func)
		{
			printf("stFunTab[%u] fun is NULL\r\n", i);
		}

		if (0 == strlen(stFunTab[i].pName))
		{
			printf("stFunTab[%u] Name is NULL\r\n", i);
		}

		if (FUN_ARGUMENTS_MAX_SIZE < stFunTab[i].paramterNum)
		{
			printf("stFunTab[%u] ParamterNum is overflow\r\n", i);
		}

		//�����û���½������
		if (0 == strcmp(stFunTab[i].pName, USER_NAME_TESTER))
		{
			sTerminal.userIndexTab[0] = i;
		}
		else if (0 == strcmp(stFunTab[i].pName, USER_NAME_ADMIN))
		{
			sTerminal.userIndexTab[1] = i;
		}
		sTerminal.authority = sTerminal.userIndexTab[1];
	}
}

/*
* ============================================================================
* Name		: input_echo
* Function	: ������Դ���
* Input		: None
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void input_echo(void)
{
	if (sTerminal.showLen < sTerminal.recvLen)
	{
		sTerminal.OutPutCallBack(sTerminal.recvBuff[sTerminal.showLen++]);
	}
}

/*
* ============================================================================
* Name		: output_string
* Function	: �����ַ���������
* Input		: const char *pStr	�ַ�����ʼ��ַ
			  uint8_t len		�ַ�������
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void output_string(const char *pStr, uint8_t len)
{
	//ERROR("pointer is null", (pStr != NULL), return;);

	while (len--)
	{
		sTerminal.OutPutCallBack(*pStr++);
	}
	sTerminal.showLen = sTerminal.recvLen;
}

/*
* ============================================================================
* Name		: check_login_cmd
* Function	: ����Ƿ��ǵ�½����
* Input		: uint8_t index	��������
* Output	: None
* Return	: true �����û���½����		false �������û���½����
* ============================================================================
*/
static bool check_login_cmd(uint8_t index)
{
	uint8_t i = 0;
	for (i = 0; i < USER_NUMBER; i++)
	{
		if (sTerminal.userIndexTab[i] == index)
		{
			return true;
		}
	}
	return false;
}

/*
* ============================================================================
* Name		: terminal_input_predeal
* Function	: �ն�����Ԥ����
* Input		: uint8_t input	���ڶ�ȡ��������
* Output	: None
* Return	: None
* Note		: �ú������ڽ����ж��е���Ϊ��
* ============================================================================
*/
void terminal_input_predeal(uint8_t input)
{
	//ready״̬��ֱ���˳����ȴ�����������������
	if (sTerminal.flag.state == TERMINAL_READY)
	{
		return;
	}
	//����ASCII���ַ��� ���账��
	if (input > 127)
	{
		return;
	}

	//��������ַ����������ң�����
	if (sTerminal.specialCharFlag)
	{
		sTerminal.specialCharFlag++;

		switch (sTerminal.specialCharFlag)
		{
			case 2:
			{
				//���������ַ��Ŀ�����
				if (91 == input)
				{
					return;
				}
				else	//�������ַ�
				{
					//�˴�Ӧ�ò���esc�����Ĵ����źţ�����esc����Ӧ�ò���δ������ʵ�֣����˵�

					//�ָ������ַ������־��
					sTerminal.specialCharFlag = 0;
					break;
				}
			}
			case 3:
			{
				if (65 <= input && input <= 68)
				{
					sTerminal.ctrlType = CHAR_TO_SPECIAL(input);
					sTerminal.flag.state = TERMINAL_READY;
				}
				sTerminal.specialCharFlag = 0;
				return;
			}
			default:
			{
				sTerminal.specialCharFlag = 0;
				return;
			}
		}
	}

	//����ASCII������ַ�
	if (input < 32)
	{
		if (27 == input)
		{
			sTerminal.specialCharFlag = 1;
		}
		else	//����������ַ�
		{
			sTerminal.flag.state = TERMINAL_READY;
			sTerminal.ctrlType = input;
		}
	}
	else	//����ASCII���ӡ�ַ�
	{
		//�������������
		//sTerminal.OutPutCallBack(Data);

		//���ջ���������ǿ���л����������״̬
		if (RECV_BUFF_MAX_SIZE == sTerminal.recvLen)
		{
			sTerminal.flag.state = TERMINAL_READY;
			sTerminal.ctrlType = RECV_BUFF_OVERFLOW;
		}
		else
		{
			sTerminal.recvBuff[sTerminal.recvLen++] = input;
			sTerminal.flag.state = TERMINAL_BUSY;
		}
	}
}

/*
* ============================================================================
* Name		: searching_command
* Function	: �����Ѿ���������ݲ��ҷ��ϵ�ǰȨ�޵���������˵��û���½����
* Input		: None
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void searching_command(void)
{
	uint8_t i = 0;
	uint8_t nameLength;
	uint8_t matchIndex[STFUNTAB_SIZE] = {0};
	uint8_t MatchNum = 0;
	uint8_t newLen = sTerminal.recvLen;
	bool bMatchFlag = true;
	char Char;

	//���ջ�����Ϊ�գ����ش�ӡ��������
	if (0 == sTerminal.recvLen)
	{
		printf("\r\n");
		for (i = 0; i < sTerminal.authority; i++)
		{
			if (false == check_login_cmd(i))
			{
				printf("%s   ", stFunTab[i].pName);
			}
		}
		printf("\r\n");
		return;
	}

	//��������ƥ��������¼������
	for (i = 0; i < sTerminal.authority; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, sTerminal.recvBuff, sTerminal.recvLen))
		{
			if (false == check_login_cmd(i))
			{
				matchIndex[MatchNum++] = i;//�洢ƥ�����������
			}
		}
	}

	if (MatchNum > 1)
	{
		printf("\r\n");
		for (i = 0; i < MatchNum; i++)
		{
			printf("%s   ", stFunTab[matchIndex[i]].pName);
		}
		printf("\r\n");

		//������ͬ���ַ����������add1 add2������a֮�󣬰�tab��ʾadd
		while (bMatchFlag)
		{
			Char = stFunTab[matchIndex[0]].pName[newLen];
			for (i = 1; i < MatchNum; i++)
			{
				if (Char != stFunTab[matchIndex[i]].pName[newLen])
				{
					bMatchFlag = false;
					break;
				}
			}
			newLen++;
		}

		newLen--;
		strncpy(&sTerminal.recvBuff[sTerminal.recvLen], &stFunTab[matchIndex[0]].pName[sTerminal.recvLen], newLen - sTerminal.recvLen);
		sTerminal.recvLen = newLen;
		output_string(sTerminal.recvBuff, sTerminal.recvLen);
	}
	else if (1 == MatchNum)	//ƥ����������һ��
	{
		nameLength = strlen(stFunTab[matchIndex[0]].pName);
		newLen = nameLength - sTerminal.recvLen;		//�����ӵ��ַ�������
		strncpy(&sTerminal.recvBuff[sTerminal.recvLen], &stFunTab[matchIndex[0]].pName[sTerminal.recvLen], newLen);
		sTerminal.recvBuff[nameLength++] = ' '; 		//��ӿո��

		sTerminal.recvLen = nameLength;
		output_string(&sTerminal.recvBuff[sTerminal.showLen], newLen + 1);
	}
}

/*
* ============================================================================
* Name		: recv_semantic_analysis
* Function	: ������������ݷ�������
* Input		: None
* Output	: None
* Return	: true���������	false���������
* Note		: None
* ============================================================================
*/
static bool recv_semantic_analysis(void)
{
	uint8_t bState = true;
	uint8_t i, cmdLen;
	uint8_t temp;
	uint8_t head = 0, tail = 0;
	uint32_t pramter[FUN_ARGUMENTS_MAX_SIZE] = {0};	 //����������
	uint8_t paramterNum = 0;			//��������
	uint8_t cmdIndex = STFUNTAB_SIZE;	//ƥ������λ��
	if (0 == sTerminal.recvLen)	//���ݶ�Ϊ��
	{
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//��ȡ����������ַ���
	cmdLen = separate_string(&head, ' ', &tail);
	if (0 == cmdLen)
	{
		printf("input error\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//��������
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, &sTerminal.recvBuff[head], cmdLen))
		{
			cmdIndex = i; //��¼�����λ��
			break;
		}
	}
	if (STFUNTAB_SIZE == cmdIndex)	//δ�ҵ�����
	{
		printf("not support cmd\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//��������Ĳ���
	for (i = tail; i < sTerminal.recvLen; i += temp)
	{
		head = tail + 1;
		temp = separate_string(&head, ' ', &tail);
		if (0 == temp || paramterNum >  stFunTab[cmdIndex].paramterNum)
		{
			break;
		}

		if (false == string_to_uint(&sTerminal.recvBuff[head], temp, &pramter[paramterNum++]))
		{
			printf("paramater error\r\n");
			bState = false;
			goto SEMANTIC_ERROR;
		}
	}

	if (paramterNum != stFunTab[cmdIndex].paramterNum)
	{
		printf("paramter num not match\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	execute_handled(cmdIndex, pramter, paramterNum);

SEMANTIC_ERROR:
	if (sTerminal.recvLen)	//�������µ����ݳ��ȣ���λ���ռ����Ի�����
	{
		sTerminal.recvLast = sTerminal.recvLen;
	}
	sTerminal.recvLen = 0;
	sTerminal.showLen = 0;

	return bState;
}


/*
* ============================================================================
* Name		: separate_string
* Function	: ���ݷָ����Խ��յ����ݽ��зֶ�
* Input		: uint8_t *pStart �ָ���ʼλ��
			  const char chr �ָ���
* Output	: uint8_t *pEnd	�ַ����ָ�����λ��
* Return	: ƥ����ַ�������
* Note		: pEnd�Ľ���λ��Ϊ��һ���ָ���
* ============================================================================
*/
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd)
{
	//ERROR("pointer is null", (pStart != NULL && pEnd != NULL), return false;);
	//���˶���ķָ���
	while (*pStart < sTerminal.recvLen)
	{
		if (sTerminal.recvBuff[*pStart] != chr)
		{
			break;
		}
		++(*pStart);
	}

	//�����ַ���
	*pEnd = *pStart;
	while (*pEnd < sTerminal.recvLen)
	{
		if (sTerminal.recvBuff[*pEnd] == chr)
		{
			break;
		}
		++(*pEnd);
	}
	return *pEnd - *pStart;
}


/*
* ============================================================================
* Name		: string_to_uint
* Function	: ���ַ���ת��Ϊ����
* Input		: char *pStr �ַ�����ʼ��ַ
			  uint8_t len	�ַ�������
* Output	: uint32_t *pValue ת�������ֵ
* Return	: true �ַ���ת���ɹ�  false ת������
* Note		: ��֧��16λ���ƺ�10����������
* ============================================================================
*/
static bool string_to_uint(char *pStr, uint8_t len, uint32_t *pValue)
{
	uint32_t baseValue = 10;
	uint32_t charValue;
	bool bHexFlag = false;

	//ERROR("pointer is null", (pStr != NULL && pValue != NULL), return false;);

	if ('0' == pStr[0] && ('x' == pStr[1] || 'X' == pStr[1]))
	{
		if (len > 10)	//�������
		{
			return false;
		}
		len -= 2;
		pStr += 2;
		baseValue = 16;
		bHexFlag = true;
	}

	*pValue = 0;

	if (false == bHexFlag)
	{
		while (len--)
		{
			if (*pStr > '9' || *pStr < '0')
			{
				return false;
			}
			*pValue *= baseValue;
			*pValue += *pStr - '0';
			pStr++;
		}
	}
	else
	{
		while (len--)
		{
			//�������
			if (*pStr <= '9' && *pStr >= '0')
			{
				charValue = *pStr - '0';
			}
			else if (*pStr <= 'f' && *pStr >= 'a')
			{
				charValue = *pStr - 'a' + 10;
			}
			else if (*pStr <= 'F' && *pStr >= 'A')
			{
				charValue = *pStr - 'A' + 10;
			}
			else
			{
				return false;
			}
			//�����ý��
			*pValue *= baseValue;
			*pValue += charValue;
			pStr++;
		}
	}

	return true;
}


/*
* ============================================================================
* Name		: execute_handled
* Function	: ִ�н���������
* Input		: uint8_t funIndex 	��Ҫ���еĺ����ں����б��λ��
			  uint32_t *pArg	  	�����б�
			  uint8_t argNum		��������
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void execute_handled(uint8_t funIndex, uint32_t *pArg, uint8_t argNum)
{
	if (sTerminal.authority <= funIndex)
	{
		if (false == check_login_cmd(funIndex))
		{
			printf("Please switch user to improve authority!\r\n");
			return;
		}
	}
	switch (argNum)
	{
		case 0:
		{
			((void (*)())stFunTab[funIndex].func)();
			break;
		}
		case 1:
		{
			((void (*)(uint32_t))stFunTab[funIndex].func)(pArg[0]);
			break;
		}
		case 2:
		{
			((void (*)(uint32_t, uint32_t))stFunTab[funIndex].func)(pArg[0], pArg[1]);
			break;
		}
		case 3:
		{
			((void (*)(uint32_t, uint32_t, uint32_t))stFunTab[funIndex].func)(pArg[0], pArg[1], pArg[2]);
			break;
		}
		case 4:
		{
			((void (*)(uint32_t, uint32_t, uint32_t, uint32_t))stFunTab[funIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3]);
			break;
		}
		case 5:
		{
			((void (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))stFunTab[funIndex].func)(pArg[0], pArg[1], pArg[2],
					pArg[3], pArg[4]);
			break;
		}
	}
}

/*
* ============================================================================
* Name		: tester_login
* Function	: �����û���½У�麯��
* Input		: uint32_t key 		�û���Կ
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void tester_login(uint32_t key)
{
	if (USER_KEY_TESETER == key)
	{
		sTerminal.authority = sTerminal.userIndexTab[0];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*
* ============================================================================
* Name		: admin_login
* Function	: admin�û���½У�麯��
* Input		: uint32_t key 		�û���Կ
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void admin_login(uint32_t key)
{
	if (USER_KEY_ADMIN == key)
	{
		sTerminal.authority = sTerminal.userIndexTab[1];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*
* ============================================================================
* Name		: software_reset
* Function	: �����λ
* Input		: None
* Output	: None
* Return	: None
* Note		: ����ʵ���뵥Ƭ��ƽ̨�������
* ============================================================================
*/
static void software_reset(void)
{
	__set_FAULTMASK(1);   //STM32���������λ
	NVIC_SystemReset();
}

/*
* ============================================================================
* Name		: user_help
* Function	: �����λ
* Input		: None
* Output	: None
* Return	: None
* Note		: None
* ============================================================================
*/
static void user_help(void)
{
	printf("\r\n***************** user manual ******************\r\n");
	printf("* %-12s  %-30s *\r\n", "Tab", "show and auto fill cmd");
	printf("* %-12s  %-30s *\r\n", "Up", "show last input");
	printf("* %-12s  %-30s *\r\n", "Down", "execute last input");
	printf("* %-12s  %-30s *\r\n", "Enter", "execute cmd");
	printf("* %-12s  %-30s *\r\n", "Backspace", "delete character");
	printf("************************************************\r\n");
}
