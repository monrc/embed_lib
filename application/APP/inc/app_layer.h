#ifndef APP_LAYER_H_
#define APP_LAYER_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "print.h"
#include "console.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"


extern TaskHandle_t AppTaskCreateHandle; /* 创建任务句柄 */
extern TaskHandle_t eepromTaskHandle;
extern TaskHandle_t keyTask[];
extern TaskHandle_t TestTaskHandle;
extern TaskHandle_t Test1TaskHandle;
extern TimerHandle_t LedTimerHandle;	//周期定时器句柄



void app_init(void);

void AppTaskCreate(void *pvParameters); /* 用于创建任务 */



#endif
