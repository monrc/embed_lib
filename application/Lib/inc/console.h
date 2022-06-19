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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================================
 * Function	: 头文件依赖，外部模块
 * ============================================================================
 */
#include "print.h"
#include "led.h"
#include "debug.h"
#include "lib_port.h"



void at_24cxx_write(void);
void at_24cxx_read(void);
void test(void);


/*
 * ============================================================================
 * Function	: 配置宏定义
 * ============================================================================
 */
#define DEFAULT_AUTHORITY		0
#define RECV_BUFF_MAX_SIZE		99

#define USER_NAME_TESTER		"tester"
#define	USER_KEY_TESETER		"123"
#define USER_NAME_ADMIN			"admin"
#define	USER_KEY_ADMIN			"12345"



/*
 * ============================================================================
 * Function	: 对外接口
 * ============================================================================
 */

void console_input_predeal(uint8_t data);
void console_handler(void);
void console_init(void (*pCallBack)(uint8_t));

#endif
