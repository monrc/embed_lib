#ifndef BSP_LAYER_H
#define BSP_LAYER_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

// #include "bsp_key.h"
// #include "bsp_led.h"
// #include "bsp_time.h"
// #include "bsp_uart.h"

void nvic_init(void);
void iwdg_refresh(void);

void hard_fault_handler_c(unsigned int *hardfault_args);
void _int_hardfault_isr();

#endif
