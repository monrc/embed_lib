/********************************************************
*file name: terminal.c
*function: 解析用户输入的指令并执行，指令最多支持5个参数
*使用说明：
	1.外部接口
		terminal_init() 			终端内存初始化工作
		terminal_input_predeal() 	放在串口接收中断中（前台程序）
		terminal_handler()			放在主循环while(1)中（后台程序）
	2.按键功能
		Tab 		显示可能匹配的命令、自动匹配指令
		↑			输入历史命令
		↓			执行历史命令
		Enter		输入结束，开始执行
		Backspace	删除前一个字符
	3.添加指令
		将函数添加至stFunTab数组中，参数依次为函数名、指令名、参数个数
	4.输入示例
		指令名 空格 参数1 空格 参数2 空格 参数3 回车
		eg：test 123 345 回车
	5.移植说明
		将外部接口放在合适的位置
		重定义实现printf函数功能
		实现串口打印字符的函数，该模块中使用serial1_put_char
		添加函数指令映射表
********************************************************/

#include <string.h>
#include <stdio.h>
#include "terminal.h"


//内部函数声明
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



//函数指令映射表定义，参数为：函数、命令映射表、函数形参个数
//指令表在底部，索引号代表着权限的大小，值越大，权限越高
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

//内部终端模块结构体定义
static Terminal_t sTerminal;


/*
* ============================================================================
* Name		: terminal_handler
* Function	: 终端输入结果分析处理
* Input		: None
* Output	: None
* Return	: None
* Note		: 该函数适合放在后台程序中调用
* ============================================================================
*/
void terminal_handler(void)
{
	const char backSpace[3] = {'\b', ' ', '\b'};

	//起着同步与互斥的作用
	if (TERMINAL_READY != sTerminal.flag.state)
	{
		input_echo();	//输入回显
		return;
	}

	switch (sTerminal.ctrlType)
	{
		case '\b':	//backspace 按键
		{
			sTerminal.recvLen = sTerminal.recvLen ? sTerminal.recvLen - 1 : 0;
			output_string(backSpace, 3);
			break;
		}

		case '\t':	//Tab 按键
		{
			searching_command();
			break;
		}

		case '\r':	//Enter 按键
		{
			printf("\r\n");
			recv_semantic_analysis();
			break;
		}

		case INPUT_KEY_UP:	//↑按键
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

		case INPUT_KEY_DOWN: //↓按键
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

		case RECV_BUFF_OVERFLOW:	//缓冲区溢出
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
* Function	: 终端模块参数初始化
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
	sTerminal.OutPutCallBack = pCallBack; //串口打印函数
	sTerminal.flag.state = TERMINAL_IDEL;
	sTerminal.authority = DEFAULT_AUTHORITY;

	//函数列表检测
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

		//更新用户登陆索引表
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
* Function	: 输入回显处理
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
* Function	: 输入字符串至串口
* Input		: const char *pStr	字符串起始地址
			  uint8_t len		字符串长度
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
* Function	: 检测是否是登陆命令
* Input		: uint8_t index	命令索引
* Output	: None
* Return	: true 属于用户登陆命令		false 不属于用户登陆命令
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
* Function	: 终端输入预处理
* Input		: uint8_t input	串口读取到的数据
* Output	: None
* Return	: None
* Note		: 该函数串口接收中断中调用为佳
* ============================================================================
*/
void terminal_input_predeal(uint8_t input)
{
	//ready状态，直接退出，等待分析函数分析命令
	if (sTerminal.flag.state == TERMINAL_READY)
	{
		return;
	}
	//扩充ASCII码字符集 不予处理
	if (input > 127)
	{
		return;
	}

	//特殊控制字符（上下左右）处理
	if (sTerminal.specialCharFlag)
	{
		sTerminal.specialCharFlag++;

		switch (sTerminal.specialCharFlag)
		{
			case 2:
			{
				//存在特殊字符的可能性
				if (91 == input)
				{
					return;
				}
				else	//非特殊字符
				{
					//此处应该产生esc按键的处理信号，由于esc按键应用层暂未做功能实现，故滤掉

					//恢复特殊字符处理标志字
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

	//处理ASCII码控制字符
	if (input < 32)
	{
		if (27 == input)
		{
			sTerminal.specialCharFlag = 1;
		}
		else	//非特殊控制字符
		{
			sTerminal.flag.state = TERMINAL_READY;
			sTerminal.ctrlType = input;
		}
	}
	else	//处理ASCII码打印字符
	{
		//输入回显至窗口
		//sTerminal.OutPutCallBack(Data);

		//接收缓冲区满，强制切换至接收完成状态
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
* Function	: 根据已经输入的内容查找符合当前权限的命令，并过滤掉用户登陆命令
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

	//接收缓冲区为空，返回打印所有命令
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

	//遍历查找匹配的命令，记录索引号
	for (i = 0; i < sTerminal.authority; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, sTerminal.recvBuff, sTerminal.recvLen))
		{
			if (false == check_login_cmd(i))
			{
				matchIndex[MatchNum++] = i;//存储匹配的命令索引
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

		//查找相同的字符，如有命令：add1 add2，输入a之后，按tab显示add
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
	else if (1 == MatchNum)	//匹配的命令仅有一个
	{
		nameLength = strlen(stFunTab[matchIndex[0]].pName);
		newLen = nameLength - sTerminal.recvLen;		//新增加的字符串长度
		strncpy(&sTerminal.recvBuff[sTerminal.recvLen], &stFunTab[matchIndex[0]].pName[sTerminal.recvLen], newLen);
		sTerminal.recvBuff[nameLength++] = ' '; 		//添加空格符

		sTerminal.recvLen = nameLength;
		output_string(&sTerminal.recvBuff[sTerminal.showLen], newLen + 1);
	}
}

/*
* ============================================================================
* Name		: recv_semantic_analysis
* Function	: 根据输入的内容分析语义
* Input		: None
* Output	: None
* Return	: true：输入合理	false：输入错误
* Note		: None
* ============================================================================
*/
static bool recv_semantic_analysis(void)
{
	uint8_t bState = true;
	uint8_t i, cmdLen;
	uint8_t temp;
	uint8_t head = 0, tail = 0;
	uint32_t pramter[FUN_ARGUMENTS_MAX_SIZE] = {0};	 //参数缓冲区
	uint8_t paramterNum = 0;			//参数个数
	uint8_t cmdIndex = STFUNTAB_SIZE;	//匹配命令位置
	if (0 == sTerminal.recvLen)	//数据段为空
	{
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//获取输入的命令字符串
	cmdLen = separate_string(&head, ' ', &tail);
	if (0 == cmdLen)
	{
		printf("input error\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//查找命令
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, &sTerminal.recvBuff[head], cmdLen))
		{
			cmdIndex = i; //记录命令的位置
			break;
		}
	}
	if (STFUNTAB_SIZE == cmdIndex)	//未找到命令
	{
		printf("not support cmd\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//处理输入的参数
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
	if (sTerminal.recvLen)	//缓存最新的数据长度，复位接收及回显缓冲区
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
* Function	: 根据分隔符对接收的数据进行分段
* Input		: uint8_t *pStart 分隔开始位置
			  const char chr 分隔符
* Output	: uint8_t *pEnd	字符串分隔结束位置
* Return	: 匹配的字符串长度
* Note		: pEnd的结束位置为第一个分隔符
* ============================================================================
*/
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd)
{
	//ERROR("pointer is null", (pStart != NULL && pEnd != NULL), return false;);
	//过滤多余的分隔符
	while (*pStart < sTerminal.recvLen)
	{
		if (sTerminal.recvBuff[*pStart] != chr)
		{
			break;
		}
		++(*pStart);
	}

	//查找字符串
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
* Function	: 把字符串转化为数字
* Input		: char *pStr 字符串起始地址
			  uint8_t len	字符串长度
* Output	: uint32_t *pValue 转换后的数值
* Return	: true 字符串转换成功  false 转换出错
* Note		: 仅支持16位进制和10进制正整数
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
		if (len > 10)	//长度溢出
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
			//参数检查
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
			//运算获得结果
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
* Function	: 执行解析的命令
* Input		: uint8_t funIndex 	需要运行的函数在函数列表的位置
			  uint32_t *pArg	  	参数列表
			  uint8_t argNum		参数个数
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
* Function	: 测试用户登陆校验函数
* Input		: uint32_t key 		用户密钥
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
* Function	: admin用户登陆校验函数
* Input		: uint32_t key 		用户密钥
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
* Function	: 软件复位
* Input		: None
* Output	: None
* Return	: None
* Note		: 函数实现与单片机平台密切相关
* ============================================================================
*/
static void software_reset(void)
{
	__set_FAULTMASK(1);   //STM32程序软件复位
	NVIC_SystemReset();
}

/*
* ============================================================================
* Name		: user_help
* Function	: 软件复位
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
