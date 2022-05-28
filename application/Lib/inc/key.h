
#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 *	按键类型宏定义
 * ============================================================
 */
#define KEY_SETTLE	 0
#define SHORT_CLICK	 1 //单击并释放
#define LONG_CLICK	 2 //长按
#define DOUBLE_CLICK 3 //双击并释放
#define THRICE_CLICK 4 //三击并释放

#define TAP_PRESS	 5 //按下了
#define TAP_RELEASE	 6 //松开了
#define TAP_PRESST	 7 //按下之后每隔N时间返回一个结果+时间，
#define TAP_RELEASET 8 //按下之后释放了+时间

/*
 * ============================================================
 *	按键数组索引
 * ============================================================
 */
#define KEY0_INDEX	 0
#define KEY1_INDEX	 1
#define KEY2_INDEX	 2
#define KEY_UP_INDEX 3

/*
 * ============================================================
 *	按键属性宏定义
 * ============================================================
 */
#define KEY0_PORT		   GPIOE
#define KEY0_PIN		   GPIO_PIN_4
#define KEY0_DEFAULT_STATE GPIO_PIN_SET

#define KEY1_PORT		   GPIOE
#define KEY1_PIN		   GPIO_PIN_3
#define KEY1_DEFAULT_STATE GPIO_PIN_SET

#define KEY2_PORT		   GPIOE
#define KEY2_PIN		   GPIO_PIN_2
#define KEY2_DEFAULT_STATE GPIO_PIN_SET

#define KEY_UP_PORT			 GPIOA
#define KEY_UP_PIN			 GPIO_PIN_0
#define KEY_UP_DEFAULT_STATE GPIO_PIN_RESET

void gpio_irq_handle(uint32_t index);

void key_init(void);

void set_click_key(uint8_t id, uint16_t time, uint16_t timeout, uint16_t longpress);

void set_tap_key(uint8_t id, uint16_t press, uint16_t release, uint16_t period);

void increase_count(uint8_t index);

void click_key_deal(uint8_t id, uint32_t *pTime);


#endif
