

#include <string.h>
#include <stdio.h>
#include "console.h"

typedef struct
{
	void *func;
	const char *name;
	const uint8_t paramterNum;
} CommandTab_t;

#define USER_NUMBER		 2 //�û�����
#define STRING_PARAMETER 6

static void tester_login(char *str, uint8_t size);
static void admin_login(char *str, uint8_t size);
static void user_help(void);

//����ָ��ӳ����壬����Ϊ������������ӳ��������βθ���
//ָ����ڵײ��������Ŵ�����Ȩ�޵Ĵ�С��ֵԽ��Ȩ��Խ��
const CommandTab_t sFunTab[] = 
{
	{user_help, "?", 0},
	{software_reset, "reboot", 0},
	{tester_login, USER_NAME_TESTER, STRING_PARAMETER},
	
	{at_24cxx_read, "eepromread", 0},
	{at_24cxx_write, "eepromwrite", 0},
	
	{test, "test", 0},
	{led_test, "setled", 4},

	{admin_login, USER_NAME_ADMIN, STRING_PARAMETER},
};

#ifdef debug
#define printf debug
#endif

#define STFUNTAB_SIZE  (sizeof(sFunTab) / sizeof(sFunTab[0]))
#define RECV_BUFF_SIZE (RECV_BUFF_MAX_SIZE + 1)
#define ROW_MAX_SIZE	80
#define CMD_GAP_SIZE	5

#define RECV_BUFF_OVERFLOW 32
#define INPUT_KEY_UP	   33
#define INPUT_KEY_DOWN	   34
#define INPUT_KEY_RIGHT	   35
#define INPUT_KEY_LEFT	   36
#define CHAR_TO_SPECIAL(x) ((x)-65 + 33)

typedef void (*fun1)(uint32_t);
typedef void (*fun2)(uint32_t, uint32_t);
typedef void (*fun3)(uint32_t, uint32_t, uint32_t);
typedef void (*fun4)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef void (*fun5)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

typedef enum
{
	TERMINAL_IDEL,	//����״̬,�ȴ������һ���ַ�
	TERMINAL_BUSY,	//������,�ȴ��������
	TERMINAL_READY, //�����˿��Ʒ����ȴ���������
} State_t;

typedef struct
{
	void (*putchar)(uint8_t data);	   //��ӡ���ݻص�����
	char buff[RECV_BUFF_SIZE];		   //���ջ�����
	uint8_t rxLen;					   //���ճ���
	uint8_t rxLenBak;				   //���յ����ݻ������
	uint8_t showLen;				   //������Գ���
	uint8_t ctrlKey;				   //�����ַ�����
	uint8_t authorityTab[USER_NUMBER]; //�û���½��������������е�����
	uint8_t authority;				   //����Ȩ�޵ȼ�����ͬ�û���ͬ��������
	uint8_t extendKey;				   //������Ʒ��ű�־
	volatile State_t state;			   //����״̬����״̬��־
} Console_t;

//�ڲ���������
static void searching_command(void);
static uint8_t separate_string(uint8_t *start, const char chr, uint8_t *end);
static bool string_to_uint(char *str, uint8_t len, uint32_t *pValue);
static void execute_handled(uint8_t funIndex, uint32_t *pArg, uint8_t ArgNum);
static bool semantic_analysis(void);
static void input_echo(void);
static void output_string(const char *pStr, uint8_t len);
static bool check_login_cmd(uint8_t index);
static void print_cmd_list(void);

//�ڲ��ն�ģ��ṹ�嶨��
static Console_t sConsole;


/*
 * ============================================================================
 * Function	: �ն���������������
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void console_handler(void)
{
	const char backSpace[3] = {'\b', ' ', '\b'};

	//����ͬ���뻥�������
	if (TERMINAL_READY != sConsole.state)
	{
		input_echo(); //�������
		return;
	}

	switch (sConsole.ctrlKey)
	{
		case '\b': // backspace ����
		{
			sConsole.rxLen = sConsole.rxLen ? sConsole.rxLen - 1 : 0;
			output_string(backSpace, 3);
			break;
		}

		case '\t': // Tab ����
		{
			searching_command();
			break;
		}

		case '\r': // Enter ����
		{
			printf("\r\n");
			semantic_analysis();
			break;
		}

		case INPUT_KEY_UP: //������
		{
			if (sConsole.rxLen == 0)
			{
				if (' ' == sConsole.buff[sConsole.rxLenBak - 1])
				{
					sConsole.rxLenBak--;
				}

				sConsole.rxLen = sConsole.rxLenBak;
				output_string(sConsole.buff, sConsole.rxLen);
			}
			break;
		}

		case INPUT_KEY_DOWN: //������
		{
			if (sConsole.rxLen == 0)
			{
				sConsole.rxLen = sConsole.rxLenBak;
				output_string(sConsole.buff, sConsole.rxLen);

				printf("\r\n");
				semantic_analysis();
			}
			break;
		}

		case RECV_BUFF_OVERFLOW: //���������
		{
			printf("uart recv over flow\r\n");
			sConsole.rxLenBak = sConsole.rxLen;
			sConsole.rxLen = 0;
			break;
		}
		default:
			break;
	}
	sConsole.state = TERMINAL_IDEL;
}

/*
 * ============================================================================
 * Function	: �ն�ģ�������ʼ��
 * Input	: void *putchar ��ӡ�ַ�����ָ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void console_init(void (*putchar)(uint8_t))
{
	uint8_t i;
	memset(&sConsole, 0, sizeof(Console_t));
	sConsole.putchar = putchar; //���ڴ�ӡ����
	sConsole.state = TERMINAL_IDEL;
	sConsole.authority = DEFAULT_AUTHORITY;

	//�����б���
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (NULL == sFunTab[i].func)
		{
			printf("sFunTab[%u] fun is NULL\r\n", i);
		}

		if (0 == strlen(sFunTab[i].name))
		{
			printf("sFunTab[%u] Name is NULL\r\n", i);
		}

		if (STRING_PARAMETER < sFunTab[i].paramterNum)
		{
			printf("sFunTab[%u] ParamterNum is overflow\r\n", i);
		}

		//�����û���½������
		if (0 == strcmp(sFunTab[i].name, USER_NAME_TESTER))
		{
			sConsole.authorityTab[0] = i;
		}
		else if (0 == strcmp(sFunTab[i].name, USER_NAME_ADMIN))
		{
			sConsole.authorityTab[1] = i;
		}
		sConsole.authority = sConsole.authorityTab[1];
	}
}

/*
 * ============================================================================
 * Function	: ������Դ���
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void input_echo(void)
{
	if (sConsole.showLen < sConsole.rxLen)
	{
		sConsole.putchar(sConsole.buff[sConsole.showLen++]);
	}
}

/*
 * ============================================================================
 * Function	: �����ַ���������
 * Input	: const char *pStr	�ַ�����ʼ��ַ
			  uint8_t len		�ַ�������
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void output_string(const char *pStr, uint8_t len)
{
	while (len--)
	{
		sConsole.putchar(*pStr++);
	}
	sConsole.showLen = sConsole.rxLen;
}


/*
 * ============================================================================
 * Function	: ����Ƿ��ǵ�½����
 * Input	: uint8_t index	��������
 * Output	: None
 * Return	: true �����û���½����		false �������û���½����
 * ============================================================================
 */
static bool check_login_cmd(uint8_t index)
{
	uint8_t i = 0;
	for (i = 0; i < USER_NUMBER; i++)
	{
		if (sConsole.authorityTab[i] == index)
		{
			return true;
		}
	}
	return false;
}


/*
 * ============================================================================
 * Function	: �ն�����Ԥ����������ʾ�ַ��������ַ�
 * Input	: uint8_t input	���ڶ�ȡ��������
 * Output	: None
 * Return	: None
 * Note		: �ú������ڽ����ж��е���Ϊ��
 * ============================================================================
 */
void console_input_predeal(uint8_t input)
{
	// ready״̬��ֱ���˳����ȴ�����������������
	if (sConsole.state == TERMINAL_READY)
	{
		return;
	}
	//����ASCII���ַ��� ���账��
	if (input > 127)
	{
		return;
	}

	//��������ַ����������ң�����
	if (sConsole.extendKey)
	{
		sConsole.extendKey++;

		switch (sConsole.extendKey)
		{
			case 2:
			{
				//���������ַ��Ŀ�����
				if (91 == input)
				{
					return;
				}
				else //�������ַ�
				{
					//�˴�Ӧ�ò���esc�����Ĵ����źţ�����esc����Ӧ�ò���δ������ʵ�֣����˵�

					//�ָ������ַ������־��
					sConsole.extendKey = 0;
					break;
				}
			}
			case 3:
			{
				if (65 <= input && input <= 68)
				{
					sConsole.ctrlKey = CHAR_TO_SPECIAL(input);
					sConsole.state = TERMINAL_READY;
				}
				sConsole.extendKey = 0;
				return;
			}
			default:
			{
				sConsole.extendKey = 0;
				return;
			}
		}
	}

	//����ASCII������ַ�
	if (input < 32)
	{
		if (27 == input)
		{
			sConsole.extendKey = 1;
		}
		else //����������ַ�
		{
			sConsole.state = TERMINAL_READY;
			sConsole.ctrlKey = input;
		}
	}
	else //����ASCII���ӡ�ַ�
	{
		//�������������
		// sConsole.OutPutCallBack(Data);

		//���ջ���������ǿ���л����������״̬
		if (RECV_BUFF_MAX_SIZE == sConsole.rxLen)
		{
			sConsole.state = TERMINAL_READY;
			sConsole.ctrlKey = RECV_BUFF_OVERFLOW;
		}
		else
		{
			sConsole.buff[sConsole.rxLen++] = input;
			sConsole.state = TERMINAL_BUSY;
		}
	}
}

/*
 * ============================================================================
 * Function	: �����Ѿ���������ݲ��ҷ��ϵ�ǰȨ�޵���������˵��û���½���
			  �������������ڶˣ�����shell���Զ���ȫ����
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void searching_command(void)
{
	uint8_t i = 0;
	uint8_t nameLength;
	uint8_t matchIndex[STFUNTAB_SIZE] = {0};
	uint8_t matchNum = 0;
	uint8_t newLen = sConsole.rxLen;
	bool matchFlag = true;
	char chr;

	//���ջ�����Ϊ�գ����ش�ӡ��������
	if (0 == sConsole.rxLen)
	{
		printf("\r\n");
		for (i = 0; i < sConsole.authority; i++)
		{
			if (false == check_login_cmd(i))
			{
				printf("%s   ", sFunTab[i].name);
			}
		}
		printf("\r\n");
		return;
	}

	//��������ƥ��������¼������
	for (i = 0; i < sConsole.authority; i++)
	{
		if (0 == strncmp(sFunTab[i].name, sConsole.buff, sConsole.rxLen))
		{
			if (false == check_login_cmd(i))
			{
				matchIndex[matchNum++] = i; //�洢ƥ�����������
			}
		}
	}

	if (matchNum > 1)
	{
		printf("\r\n");
		for (i = 0; i < matchNum; i++)
		{
			printf("%s   ", sFunTab[matchIndex[i]].name);
		}
		printf("\r\n");

		//������ͬ���ַ����������add1 add2������a֮�󣬰�tab��ʾadd
		while (matchFlag)
		{
			chr = sFunTab[matchIndex[0]].name[newLen];
			for (i = 1; i < matchNum; i++)
			{
				if (chr != sFunTab[matchIndex[i]].name[newLen])
				{
					matchFlag = false;
					break;
				}
			}
			newLen++;
		}

		newLen--;
		strncpy(&sConsole.buff[sConsole.rxLen], &sFunTab[matchIndex[0]].name[sConsole.rxLen], newLen - sConsole.rxLen);
		sConsole.rxLen = newLen;
		output_string(sConsole.buff, sConsole.rxLen);
	}
	else if (1 == matchNum) //ƥ����������һ��
	{
		nameLength = strlen(sFunTab[matchIndex[0]].name);
		newLen = nameLength - sConsole.rxLen; //�����ӵ��ַ�������
		strncpy(&sConsole.buff[sConsole.rxLen], &sFunTab[matchIndex[0]].name[sConsole.rxLen], newLen);
		sConsole.buff[nameLength++] = ' '; //��ӿո��

		sConsole.rxLen = nameLength;
		output_string(&sConsole.buff[sConsole.showLen], newLen + 1);
	}
}


/*
 * ============================================================================
 * Function	: ������������ݷ������壬��ִ��ָ��
 * Input	: None
 * Output	: None
 * Return	: true���������	false���������
 * ============================================================================
 */
static bool semantic_analysis(void)
{
	uint8_t ret = true;
	uint8_t i, cmdLen;
	uint8_t temp;
	uint8_t head = 0, tail = 0;
	uint32_t paramter[STRING_PARAMETER] = {0}; //����������
	uint8_t paramterNum = 0;				  //��������
	uint8_t cmdIndex = STFUNTAB_SIZE;		  //ƥ������λ��
	if (0 == sConsole.rxLen)				  //���ݶ�Ϊ��
	{
		ret = false;
		goto RECORD_INPUTS;
	}

	//��ȡ����������ַ���
	cmdLen = separate_string(&head, ' ', &tail);
	if (0 == cmdLen)
	{
		printf("input error\r\n");
		ret = false;
		goto RECORD_INPUTS;
	}

	//��������
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (0 == strncmp(sFunTab[i].name, &sConsole.buff[head], cmdLen)
			&& cmdLen == strlen(sFunTab[i].name))
		{
			cmdIndex = i; //��¼�����λ��
			break;
		}
	}
	if (STFUNTAB_SIZE == cmdIndex) //δ�ҵ�����
	{
		printf("not support cmd\r\n");
		ret = false;
		goto RECORD_INPUTS;
	}

	if (sFunTab[cmdIndex].paramterNum == STRING_PARAMETER)
	{
		paramter[0] = tail + 1;	//�ַ�����ʼλ��
		paramter[1] = sConsole.rxLen - paramter[0]; //�ַ�������
		paramterNum = STRING_PARAMETER;
		goto EXECUTE_INPUT_CMD;
	}

	//��������Ĳ���
	for (i = tail; i < sConsole.rxLen; i += temp)
	{
		head = tail + 1;
		temp = separate_string(&head, ' ', &tail);
		if (0 == temp || paramterNum > sFunTab[cmdIndex].paramterNum)
		{
			break;
		}

		if (false == string_to_uint(&sConsole.buff[head], temp, &paramter[paramterNum++]))
		{
			printf("paramater error\r\n");
			ret = false;
			goto RECORD_INPUTS;
		}
	}

	if (paramterNum != sFunTab[cmdIndex].paramterNum)
	{
		printf("paramter num not match\r\n");
		ret = false;
		goto RECORD_INPUTS;
	}

EXECUTE_INPUT_CMD:
	execute_handled(cmdIndex, paramter, paramterNum);

RECORD_INPUTS:
	if (sConsole.rxLen) //�������µ����ݳ��ȣ���λ���ռ����Ի�����
	{
		sConsole.rxLenBak = sConsole.rxLen;
	}
	sConsole.rxLen = 0;
	sConsole.showLen = 0;

	return ret;
}

/*
 * ============================================================================
 * Function	: ���ݷָ����Խ��յ����ݽ��зֶ�
 * Input	: uint8_t *start �ַ�����ʼλ��
			  const char chr �ָ���
 * Output	: uint8_t *end	�ַ����ָ�����λ��
 * Return	: ƥ����ַ�������
 * Note		: end ��λ��Ϊ�ַ����ĵ�һ���ָ���
 * ============================================================================
 */
static uint8_t separate_string(uint8_t *start, const char chr, uint8_t *end)
{
	//���˶���ķָ���
	while (*start < sConsole.rxLen)
	{
		if (sConsole.buff[*start] != chr)
		{
			break;
		}
		++(*start);
	}

	//�����ַ���
	*end = *start;
	while (*end < sConsole.rxLen)
	{
		if (sConsole.buff[*end] == chr)
		{
			break;
		}
		++(*end);
	}
	return *end - *start;
}

/*
 * ============================================================================
 * Function	: ���ַ���ת��Ϊ����
 * Input	: char *str �ַ�����ʼ��ַ
			  uint8_t len �ַ�������
 * Output	: uint32_t *integer ת�������ֵ
 * Return	: true �ַ���ת���ɹ�  false ת������
 * Note		: ��֧��16λ���ƺ�10����������
 * ============================================================================
 */
static bool string_to_uint(char *str, uint8_t len, uint32_t *integer)
{
	uint8_t chr;

	*integer = 0;

	if ('0' == str[0] && ('x' == str[1] || 'X' == str[1]))
	{
		if (len > 10) //�������
		{
			return false;
		}

		len -= 2;
		str += 2;

		while (len--)
		{
			//�������
			if (*str <= '9' && *str >= '0')
			{
				chr = *str - '0';
			}
			else if (*str <= 'f' && *str >= 'a')
			{
				chr = *str - 'a' + 10;
			}
			else if (*str <= 'F' && *str >= 'A')
			{
				chr = *str - 'A' + 10;
			}
			else
			{
				return false;
			}
			//�����ý��
			*integer *= 16;
			*integer += chr;
			str++;
		}
	}
	else
	{
		while (len--)
		{
			if (*str > '9' || *str < '0')
			{
				return false;
			}
			*integer *= 10;
			*integer += *str - '0';
			str++;
		}
	}

	return true;
}

/*
 * ============================================================================
 * Function	: ִ�н���������
 * Input	: uint8_t funIndex 	��Ҫ���еĺ����ں����б��λ��
			  uint32_t *pArg	�����б�
			  uint8_t argNum	��������
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void execute_handled(uint8_t funIndex, uint32_t *arg, uint8_t argNum)
{
	if (sConsole.authority <= funIndex)
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
			((void (*)())sFunTab[funIndex].func)();
			break;
		}
		case 1:
		{
			((fun1)sFunTab[funIndex].func)(arg[0]);
			break;
		}
		case 2:
		{
			((fun2)sFunTab[funIndex].func)(arg[0], arg[1]);
			break;
		}
		case 3:
		{
			((fun3)sFunTab[funIndex].func)(arg[0], arg[1], arg[2]);
			break;
		}
		case 4:
		{
			((fun4)sFunTab[funIndex].func)(arg[0], arg[1], arg[2], arg[3]);
			break;
		}
		case 5:
		{
			((fun5)sFunTab[funIndex].func)(arg[0], arg[1], arg[2], arg[3], arg[4]);
			break;
		}
		case STRING_PARAMETER:
		{
			((void (*)(char *, uint8_t size))sFunTab[funIndex].func)(sConsole.buff + arg[0], arg[1]);
			break;
		}
		default:
			break;
	}
}

/*
 * ============================================================================
 * Function	: �����û���½У�麯��
 * Input	: char *str 	�������Կ
			  uint8_t size	���볤��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void tester_login(char *str, uint8_t size)
{
	if (sizeof(USER_KEY_TESETER) == size + 1
		&& memcmp(str, USER_KEY_TESETER, size) == 0)
	{
		sConsole.authority = sConsole.authorityTab[0];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*
 * ============================================================================
 * Function	: admin�û���½У�麯��
 * Input	: char *str 	�������Կ
			  uint8_t size	���볤��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void admin_login(char *str, uint8_t size)
{
	if (sizeof(USER_KEY_ADMIN) == size + 1
		&& memcmp(str, USER_KEY_ADMIN, size) == 0)
	{
		sConsole.authority = sConsole.authorityTab[1];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*
 * ============================================================================
 * Function	: ����̨ʹ��˵��
 * Input	: None
 * Output	: None
 * Return	: None
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
	
	print_cmd_list();
}


/*
 * ============================================================================
 * Function	: ��������б�
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
static void print_cmd_list(void)
{
	uint8_t i,j;
	uint8_t maxLen = 0;
	uint8_t temp;
	uint8_t row;

	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		temp = strlen(sFunTab[i].name);
		if (temp > maxLen)
		{
			maxLen = temp;
		}
	}

	row = ROW_MAX_SIZE / (maxLen + CMD_GAP_SIZE);
	maxLen += CMD_GAP_SIZE - 2;

	for (i = 0, j = 0; i < STFUNTAB_SIZE; i++)
	{
		if (true == check_login_cmd(i))
		{
			continue;
		}

		printf("%s %u", sFunTab[i].name, sFunTab[i].paramterNum);
		temp = maxLen - strlen(sFunTab[i].name);
		printf("%*s", temp, "");	//��� temp���ո�

		j++;
		if ((j % row) == 0)
		{
			printf("\r\n");
		}
	}
	printf("\r\n");
}
