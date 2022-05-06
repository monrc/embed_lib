
#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <stdbool.h>


//��־ģ��
#define BIT(x)	  ((uint32_t)1 << x)
#define LOG_PRINT BIT(0)
#define LOG_PBSP  BIT(1)
#define LOG_APP	  BIT(2)
#define LOG_UART  BIT(3)

//��־�ȼ�
#define BSP_LEVEL	  1
#define APP_LEVEL	  2
#define PRINT_LEVEL 0xff

//��־��ɫ
#define NONE   "\e[0m"	  //�����ɫ����֮��Ĵ�ӡΪ���������֮ǰ�Ĳ���Ӱ��
#define BLACK  "\e[0;30m" //���
#define RED	   "\e[0;31m" //��죬����
#define GREEN  "\e[0;32m" //���̣�����
#define BROWN  "\e[0;33m" //��ƣ�����
#define YELLOW "\e[1;33m" //�ʻ�
#define BLUE   "\e[0;34m" //����������
#define PURPLE "\e[0;35m" //��ۣ����ۣ�ƫ����
#define CYAN   "\e[0;36m" //����ɫ
#define GRAY   "\e[0;37m" //��ɫ

#define L_BLACK	 "\e[1;30m" //���ڣ�ƫ�Һ�
#define L_RED	 "\e[1;31m" //�ʺ�
#define L_GREEN	 "\e[1;32m" //����
#define L_BLUE	 "\e[1;34m" //������ƫ�׻�
#define L_PURPLE "\e[1;35m" //���ۣ�ƫ�׻�
#define L_CYAN	 "\e[1;36m" //������ɫ

#define WHITE	  "\e[1;37m" //��ɫ�������һ�㣬�������󣬱�boldС
#define BOLD	  "\e[1m"	 //��ɫ������
#define UNDERLINE "\e[4m"	 //�»��ߣ���ɫ��������С
#define BLINK	  "\e[5m"	 //��˸����ɫ��������С
#define REVERSE	  "\e[7m"	 //��ת�������屳��Ϊ��ɫ������Ϊ��ɫ
#define HIDE	  "\e[8m"	 //����
#define CLEAR	  "\e[2J"	 //���
#define CLRLINE	  "\r\e[K"	 //�����

//��־�궨��
#define debug(format, arg...)				   print(LOG_PRINT, APP_LEVEL, format, ##arg);
#define debug_r(module, level, format, arg...) print(module, level, RED format NONE, ##arg);
#define debug_g(module, level, format, arg...) print(module, level, GREEN format NONE, ##arg);
#define debug_b(module, level, format, arg...) print(module, level, BLUE format NONE, ##arg);
#define debug_y(module, level, format, arg...) print(module, level, YELLOW format NONE, ##arg);

#define debug_e(format, arg...) print(LOG_PRINT, APP_LEVEL, RED "error %s, %u" format NONE, __FILE__, __LINE__, ##arg);

#define debug_error(format, arg...)	  print_error(__FILE__, __LINE__, format, ##arg)
#define debug_assert(format, arg...)  print_assert(__FILE__, __LINE__, format, ##arg)
#define debug_warning(format, arg...) print_warning(__FILE__, __LINE__, format, ##arg)
#define debug_prompt(format, arg...)  debug_g(LOG_PRINT, PRINT_LEVEL, format, ##arg)

//��־ģ�鹦��
void debug_init(void);

void print(uint32_t module, uint8_t level, const char *format, ...);
void print_wait(const char *format, ...);

void uart_send_bytes(uint8_t *buff, uint8_t size);
void uart_send_byte(uint8_t byte);

void print_array(uint32_t module, const char *name, uint8_t *buff, uint8_t size);

void print_error(const char *file, int line, const char *format, ...);
void print_assert(const char *file, int line, const char *format, ...);
void print_warning(const char *file, int line, const char *format, ...);

void uart_send_irq(void);


#endif
