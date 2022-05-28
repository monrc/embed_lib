
#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 *	�������ͺ궨��
 * ============================================================
 */
#define KEY_SETTLE	 0
#define SHORT_CLICK	 1 //�������ͷ�
#define LONG_CLICK	 2 //����
#define DOUBLE_CLICK 3 //˫�����ͷ�
#define THRICE_CLICK 4 //�������ͷ�

#define TAP_PRESS	 5 //������
#define TAP_RELEASE	 6 //�ɿ���
#define TAP_PRESST	 7 //����֮��ÿ��Nʱ�䷵��һ�����+ʱ�䣬
#define TAP_RELEASET 8 //����֮���ͷ���+ʱ��

/*
 * ============================================================
 *	������������
 * ============================================================
 */
#define KEY0_INDEX	 0
#define KEY1_INDEX	 1
#define KEY2_INDEX	 2
#define KEY_UP_INDEX 3

/*
 * ============================================================
 *	�������Ժ궨��
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
