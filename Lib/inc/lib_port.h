
#ifndef LIB_PORT_H
#define LIB_PORT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================================
 * Function	: ������־ģ����ؽӿ�
 * ============================================================================
 */
void write_uart_tx_reg(uint8_t byte);
void enable_uart_tx_irq(void);
void disable_uart_tx_irq(void);
bool get_uart_tx_busy(void);


/*
 * ============================================================================
 * Function	: ������ؽӿ�
 * ============================================================================
 */
uint8_t get_key_pressed(uint8_t id);

void set_led_state(uint8_t id, uint8_t state);

void input_key_deal(uint8_t id, uint8_t keyType);


#endif