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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================================
 * Function	: ͷ�ļ��������ⲿģ��
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
 * Function	: ���ú궨��
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
 * Function	: ����ӿ�
 * ============================================================================
 */

void console_input_predeal(uint8_t data);
void console_handler(void);
void console_init(void (*pCallBack)(uint8_t));

#endif
