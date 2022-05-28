
#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <stdbool.h>


//日志模块
#define BIT(x)	  ((uint32_t)1 << x)
#define LOG_PRINT BIT(0)
#define LOG_PBSP  BIT(1)
#define LOG_APP	  BIT(2)
#define LOG_UART  BIT(3)

//日志等级
#define BSP_LEVEL	  1
#define APP_LEVEL	  2
#define PRINT_LEVEL 0xff

//日志颜色
#define NONE   "\e[0m"	  //清除颜色，即之后的打印为正常输出，之前的不受影响
#define BLACK  "\e[0;30m" //深黑
#define RED	   "\e[0;31m" //深红，暗红
#define GREEN  "\e[0;32m" //深绿，暗绿
#define BROWN  "\e[0;33m" //深黄，暗黄
#define YELLOW "\e[1;33m" //鲜黄
#define BLUE   "\e[0;34m" //深蓝，暗蓝
#define PURPLE "\e[0;35m" //深粉，暗粉，偏暗紫
#define CYAN   "\e[0;36m" //暗青色
#define GRAY   "\e[0;37m" //灰色

#define L_BLACK	 "\e[1;30m" //亮黑，偏灰褐
#define L_RED	 "\e[1;31m" //鲜红
#define L_GREEN	 "\e[1;32m" //鲜绿
#define L_BLUE	 "\e[1;34m" //亮蓝，偏白灰
#define L_PURPLE "\e[1;35m" //亮粉，偏白灰
#define L_CYAN	 "\e[1;36m" //鲜亮青色

#define WHITE	  "\e[1;37m" //白色，字体粗一点，比正常大，比bold小
#define BOLD	  "\e[1m"	 //白色，粗体
#define UNDERLINE "\e[4m"	 //下划线，白色，正常大小
#define BLINK	  "\e[5m"	 //闪烁，白色，正常大小
#define REVERSE	  "\e[7m"	 //反转，即字体背景为白色，字体为黑色
#define HIDE	  "\e[8m"	 //隐藏
#define CLEAR	  "\e[2J"	 //清除
#define CLRLINE	  "\r\e[K"	 //清除行

//日志宏定义
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

//日志模块功能
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
