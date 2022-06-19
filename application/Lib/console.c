

#include <string.h>
#include <stdio.h>
#include "console.h"

typedef struct
{
	void *func;
	const char *name;
	const uint8_t paramterNum;
} CommandTab_t;

#define USER_NUMBER		 2 //用户个数
#define STRING_PARAMETER 6

static void tester_login(char *str, uint8_t size);
static void admin_login(char *str, uint8_t size);
static void user_help(void);

//函数指令映射表定义，参数为：函数、命令映射表、函数形参个数
//指令表在底部，索引号代表着权限的大小，值越大，权限越高
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
	TERMINAL_IDEL,	//空闲状态,等待输入第一个字符
	TERMINAL_BUSY,	//输入中,等待输入完成
	TERMINAL_READY, //输入了控制符，等待分析处理
} State_t;

typedef struct
{
	void (*putchar)(uint8_t data);	   //打印数据回调函数
	char buff[RECV_BUFF_SIZE];		   //接收缓冲区
	uint8_t rxLen;					   //接收长度
	uint8_t rxLenBak;				   //接收的数据缓存个数
	uint8_t showLen;				   //输入回显长度
	uint8_t ctrlKey;				   //控制字符类型
	uint8_t authorityTab[USER_NUMBER]; //用户登陆索引表，在命令表中的索引
	uint8_t authority;				   //访问权限等级，不同用户不同访问限制
	uint8_t extendKey;				   //特殊控制符号标志
	volatile State_t state;			   //接收状态处理状态标志
} Console_t;

//内部函数声明
static void searching_command(void);
static uint8_t separate_string(uint8_t *start, const char chr, uint8_t *end);
static bool string_to_uint(char *str, uint8_t len, uint32_t *pValue);
static void execute_handled(uint8_t funIndex, uint32_t *pArg, uint8_t ArgNum);
static bool semantic_analysis(void);
static void input_echo(void);
static void output_string(const char *pStr, uint8_t len);
static bool check_login_cmd(uint8_t index);
static void print_cmd_list(void);

//内部终端模块结构体定义
static Console_t sConsole;


/*
 * ============================================================================
 * Function	: 终端输入结果分析处理
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void console_handler(void)
{
	const char backSpace[3] = {'\b', ' ', '\b'};

	//起着同步与互斥的作用
	if (TERMINAL_READY != sConsole.state)
	{
		input_echo(); //输入回显
		return;
	}

	switch (sConsole.ctrlKey)
	{
		case '\b': // backspace 按键
		{
			sConsole.rxLen = sConsole.rxLen ? sConsole.rxLen - 1 : 0;
			output_string(backSpace, 3);
			break;
		}

		case '\t': // Tab 按键
		{
			searching_command();
			break;
		}

		case '\r': // Enter 按键
		{
			printf("\r\n");
			semantic_analysis();
			break;
		}

		case INPUT_KEY_UP: //↑按键
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

		case INPUT_KEY_DOWN: //↓按键
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

		case RECV_BUFF_OVERFLOW: //缓冲区溢出
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
 * Function	: 终端模块参数初始化
 * Input	: void *putchar 打印字符函数指针
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void console_init(void (*putchar)(uint8_t))
{
	uint8_t i;
	memset(&sConsole, 0, sizeof(Console_t));
	sConsole.putchar = putchar; //串口打印函数
	sConsole.state = TERMINAL_IDEL;
	sConsole.authority = DEFAULT_AUTHORITY;

	//函数列表检测
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

		//更新用户登陆索引表
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
 * Function	: 输入回显处理
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
 * Function	: 输入字符串至串口
 * Input	: const char *pStr	字符串起始地址
			  uint8_t len		字符串长度
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
 * Function	: 检测是否是登陆命令
 * Input	: uint8_t index	命令索引
 * Output	: None
 * Return	: true 属于用户登陆命令		false 不属于用户登陆命令
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
 * Function	: 终端输入预处理，区分显示字符、控制字符
 * Input	: uint8_t input	串口读取到的数据
 * Output	: None
 * Return	: None
 * Note		: 该函数串口接收中断中调用为佳
 * ============================================================================
 */
void console_input_predeal(uint8_t input)
{
	// ready状态，直接退出，等待分析函数分析命令
	if (sConsole.state == TERMINAL_READY)
	{
		return;
	}
	//扩充ASCII码字符集 不予处理
	if (input > 127)
	{
		return;
	}

	//特殊控制字符（上下左右）处理
	if (sConsole.extendKey)
	{
		sConsole.extendKey++;

		switch (sConsole.extendKey)
		{
			case 2:
			{
				//存在特殊字符的可能性
				if (91 == input)
				{
					return;
				}
				else //非特殊字符
				{
					//此处应该产生esc按键的处理信号，由于esc按键应用层暂未做功能实现，故滤掉

					//恢复特殊字符处理标志字
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

	//处理ASCII码控制字符
	if (input < 32)
	{
		if (27 == input)
		{
			sConsole.extendKey = 1;
		}
		else //非特殊控制字符
		{
			sConsole.state = TERMINAL_READY;
			sConsole.ctrlKey = input;
		}
	}
	else //处理ASCII码打印字符
	{
		//输入回显至窗口
		// sConsole.OutPutCallBack(Data);

		//接收缓冲区满，强制切换至接收完成状态
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
 * Function	: 根据已经输入的内容查找符合当前权限的命令，并过滤掉用户登陆命令，
			  将结果输出至串口端，类似shell的自动补全功能
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

	//接收缓冲区为空，返回打印所有命令
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

	//遍历查找匹配的命令，记录索引号
	for (i = 0; i < sConsole.authority; i++)
	{
		if (0 == strncmp(sFunTab[i].name, sConsole.buff, sConsole.rxLen))
		{
			if (false == check_login_cmd(i))
			{
				matchIndex[matchNum++] = i; //存储匹配的命令索引
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

		//查找相同的字符，如有命令：add1 add2，输入a之后，按tab显示add
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
	else if (1 == matchNum) //匹配的命令仅有一个
	{
		nameLength = strlen(sFunTab[matchIndex[0]].name);
		newLen = nameLength - sConsole.rxLen; //新增加的字符串长度
		strncpy(&sConsole.buff[sConsole.rxLen], &sFunTab[matchIndex[0]].name[sConsole.rxLen], newLen);
		sConsole.buff[nameLength++] = ' '; //添加空格符

		sConsole.rxLen = nameLength;
		output_string(&sConsole.buff[sConsole.showLen], newLen + 1);
	}
}


/*
 * ============================================================================
 * Function	: 根据输入的内容分析语义，并执行指令
 * Input	: None
 * Output	: None
 * Return	: true：输入合理	false：输入错误
 * ============================================================================
 */
static bool semantic_analysis(void)
{
	uint8_t ret = true;
	uint8_t i, cmdLen;
	uint8_t temp;
	uint8_t head = 0, tail = 0;
	uint32_t paramter[STRING_PARAMETER] = {0}; //参数缓冲区
	uint8_t paramterNum = 0;				  //参数个数
	uint8_t cmdIndex = STFUNTAB_SIZE;		  //匹配命令位置
	if (0 == sConsole.rxLen)				  //数据段为空
	{
		ret = false;
		goto RECORD_INPUTS;
	}

	//获取输入的命令字符串
	cmdLen = separate_string(&head, ' ', &tail);
	if (0 == cmdLen)
	{
		printf("input error\r\n");
		ret = false;
		goto RECORD_INPUTS;
	}

	//查找命令
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (0 == strncmp(sFunTab[i].name, &sConsole.buff[head], cmdLen)
			&& cmdLen == strlen(sFunTab[i].name))
		{
			cmdIndex = i; //记录命令的位置
			break;
		}
	}
	if (STFUNTAB_SIZE == cmdIndex) //未找到命令
	{
		printf("not support cmd\r\n");
		ret = false;
		goto RECORD_INPUTS;
	}

	if (sFunTab[cmdIndex].paramterNum == STRING_PARAMETER)
	{
		paramter[0] = tail + 1;	//字符串开始位置
		paramter[1] = sConsole.rxLen - paramter[0]; //字符串长度
		paramterNum = STRING_PARAMETER;
		goto EXECUTE_INPUT_CMD;
	}

	//处理输入的参数
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
	if (sConsole.rxLen) //缓存最新的数据长度，复位接收及回显缓冲区
	{
		sConsole.rxLenBak = sConsole.rxLen;
	}
	sConsole.rxLen = 0;
	sConsole.showLen = 0;

	return ret;
}

/*
 * ============================================================================
 * Function	: 根据分隔符对接收的数据进行分段
 * Input	: uint8_t *start 字符串起始位置
			  const char chr 分隔符
 * Output	: uint8_t *end	字符串分隔结束位置
 * Return	: 匹配的字符串长度
 * Note		: end 的位置为字符串的第一个分隔符
 * ============================================================================
 */
static uint8_t separate_string(uint8_t *start, const char chr, uint8_t *end)
{
	//过滤多余的分隔符
	while (*start < sConsole.rxLen)
	{
		if (sConsole.buff[*start] != chr)
		{
			break;
		}
		++(*start);
	}

	//查找字符串
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
 * Function	: 把字符串转化为数字
 * Input	: char *str 字符串起始地址
			  uint8_t len 字符串长度
 * Output	: uint32_t *integer 转换后的数值
 * Return	: true 字符串转换成功  false 转换出错
 * Note		: 仅支持16位进制和10进制正整数
 * ============================================================================
 */
static bool string_to_uint(char *str, uint8_t len, uint32_t *integer)
{
	uint8_t chr;

	*integer = 0;

	if ('0' == str[0] && ('x' == str[1] || 'X' == str[1]))
	{
		if (len > 10) //长度溢出
		{
			return false;
		}

		len -= 2;
		str += 2;

		while (len--)
		{
			//参数检查
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
			//运算获得结果
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
 * Function	: 执行解析的命令
 * Input	: uint8_t funIndex 	需要运行的函数在函数列表的位置
			  uint32_t *pArg	参数列表
			  uint8_t argNum	参数个数
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
 * Function	: 测试用户登陆校验函数
 * Input	: char *str 	输入的密钥
			  uint8_t size	密码长度
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
 * Function	: admin用户登陆校验函数
 * Input	: char *str 	输入的密钥
			  uint8_t size	密码长度
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
 * Function	: 控制台使用说明
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
 * Function	: 输出命令列表
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
		printf("%*s", temp, "");	//输出 temp个空格

		j++;
		if ((j % row) == 0)
		{
			printf("\r\n");
		}
	}
	printf("\r\n");
}
