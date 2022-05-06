
#ifndef LIB_PORT_H
#define LIB_PORT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "led.h"

/*
 * ============================================================================
 * Function	: 串口日志模块相关接口
 * ============================================================================
 */
void write_uart_tx_reg(uint8_t byte);
void enable_uart_tx_irq(void);
void disable_uart_tx_irq(void);
bool get_uart_tx_busy(void);


/*
 * ============================================================================
 * Function	: 按键相关接口
 * ============================================================================
 */
 typedef struct
{
	uint8_t id;
	uint16_t scanTime;
	uint16_t timeOut;
	uint16_t longPress;
}KeyTaskParamter_t;

uint8_t get_key_pressed(uint8_t id);
void key_irq_handle(TaskHandle_t keyTask, uint8_t id);
void input_key_deal(uint8_t id, uint8_t keyType);
void key_task(void *parameter);

/*
 * ============================================================================
 * Function	: 调试串口相关接口
 * ============================================================================
 */
void terminal_irq_handle(uint8_t input);
void terminal_task(void *parameter);

/*
 * ============================================================================
 * Function	: io端口相关接口，LED灯、蜂鸣器
 * ============================================================================
 */
void set_io_output(uint8_t id, uint8_t state);
void led_task(void *parameter);
void start_led_task(IODEVICE_t id, uint16_t onTime, uint16_t offTime, uint8_t repeat);



#endif