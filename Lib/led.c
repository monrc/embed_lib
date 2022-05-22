
#include <string.h>
#include "led.h"
#include "lib_port.h"

#define CRITICAL_PROTECT 1 // 0 无临界保护、 1 任务级临界保护、 2 中断级临界保护

#if (CRITICAL_PROTECT == 2)
#include "FreeRTOS.h"
#define enter_critical() uint32_t interrupt = portSET_INTERRUPT_MASK_FROM_ISR();
#define exit_critical()	 portCLEAR_INTERRUPT_MASK_FROM_ISR(interrupt)
#elif (CRITICAL_PROTECT == 1)
#include "FreeRTOS.h"
#define enter_critical() portENTER_CRITICAL()
#define exit_critical()	 portEXIT_CRITICAL()
#else
#define enter_critical()
#define exit_critical()
#endif

#define MAX_WAIT_TIME		0xffffffff
#define LED_NUM				(sizeof(funList) / sizeof(funList[0]))
#define CLOSE_LED			0
#define OPEN_LED			1

typedef void (*func_t)(uint8_t);
typedef enum
{
	LED_IDLE,
	LED_ON,
	LED_OFF,
	LED_END
} LedStep_t;

typedef struct
{
	LedStep_t step;
	uint8_t count;
	uint8_t onCount;
	uint8_t offCount;
	uint8_t repeat;
} Led_t;

static void set_green(uint8_t state);
static void set_blue(uint8_t state);
static void set_beeper(uint8_t state);
static void set_leds(uint8_t id, uint8_t state);

func_t funList[] = {
	set_green,
	set_blue,
	set_beeper,
};

Led_t sLed[LED_NUM];

/*
 * ============================================================================
 * Function	: LED初始化
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void led_init(void)
{
	uint8_t i;
	memset(sLed, 0, sizeof(sLed));
	for (i = 0; i < LED_NUM; i++)
	{
		set_leds(i, CLOSE_LED);
	}
}

/*
 * ============================================================================
 * Function	: 设置IO输出接口
 * ============================================================================
 */
static void set_green(uint8_t state)
{
	set_io_output(0, state);
}

static void set_blue(uint8_t state)
{
	set_io_output(1, state);
}

static void set_beeper(uint8_t state)
{
	set_io_output(2, state);
}

static void set_leds(uint8_t id, uint8_t state)
{
	funList[id](state);
}

/*
 * ============================================================================
 * Function	: 设置IO类输出方式
 * Input	: uint8_t id 设备ID
			  uint8_t onCount 开启时间
			  uint8_t offCount 关闭时间
			  uint8_t repeat 重复次数
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void set_output(IODEVICE_t id, uint8_t onCount, uint8_t offCount, uint8_t repeat)
{
	if (id >= LED_NUM)
	{
		return;
	}

	enter_critical();

	set_leds(id, CLOSE_LED);

	if (onCount && repeat)
	{
		sLed[id].count = onCount;
		sLed[id].onCount = onCount;
		sLed[id].offCount = offCount;
		sLed[id].repeat = repeat;
		sLed[id].step = LED_ON;
		set_leds(id, OPEN_LED);
	}
	else
	{
		sLed[id].step = LED_IDLE;
	}

	exit_critical();
}

/*
 * ============================================================================
 * Function	: io 类设备定期执行函数,输出闪烁或者常亮灯控
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void leds_output_deal(void)
{
	uint8_t i;
	for (i = 0; i < LED_NUM; i++)
	{
		if (sLed[i].step == LED_IDLE) //灯未启动
		{
			continue;
		}

		sLed[i].count--;
		if (sLed[i].count) //时间未到
		{
			continue;
		}

		if (sLed[i].step == LED_ON)
		{
			if (sLed[i].offCount) // 关灯时间非零，闪烁模式
			{
				sLed[i].step = LED_OFF;
				sLed[i].count = sLed[i].offCount;
				set_leds(i, CLOSE_LED);
			}
			else
			{
				sLed[i].step = LED_OFF; //关灯时间为零，跳过该阶段
			}
		}

		if (sLed[i].step == LED_OFF && sLed[i].count == 0)
		{
			sLed[i].repeat--;
			if (sLed[i].repeat)
			{
				if (sLed[i].repeat == (LED_FOREVER - 1)) //一直输出模式
				{
					sLed[i].repeat = LED_FOREVER;
				}
				
				sLed[i].count = sLed[i].onCount;
				sLed[i].step = LED_ON;
				set_leds(i, OPEN_LED);
			}
			else
			{
				sLed[i].step = LED_IDLE;
				set_leds(i, CLOSE_LED);
			}
		}
	}
}

/*
 * ============================================================================
 * Function	: io 输出延时时间
 * Input	: None
 * Output	: None
 * Return	: 下次 leds_output_deal() 执行的间隔时间
 * ============================================================================
 */
uint32_t get_output_wait_time(void)
{
	uint8_t i;

	for (i = 0; i < LED_NUM; i++)
	{
		if (sLed[i].step != LED_IDLE)
		{
			return LED_SCAN_TIME;
		}
	}

	return MAX_WAIT_TIME;
}

