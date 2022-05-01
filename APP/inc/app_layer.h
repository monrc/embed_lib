#ifndef APP_LAYER_H_
#define APP_LAYER_H_

#include "print.h"


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "terminal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"


extern TaskHandle_t AppTaskCreateHandle; /* ���������� */
extern TaskHandle_t LedTaskHandle;
extern TaskHandle_t KeyTaskHandle;
extern TaskHandle_t TerminalTaskHandle;
extern TaskHandle_t TestTaskHandle;
extern TaskHandle_t Test1TaskHandle;
extern TimerHandle_t LedTimerHandle;	//���ڶ�ʱ�����



void app_init(void);

void AppTaskCreate(void *pvParameters); /* ���ڴ������� */



#endif