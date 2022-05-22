
#include <string.h>
#include "led.h"
#include "lib_port.h"

#define CRITICAL_PROTECT 1 // 0 ���ٽ籣���� 1 �����ٽ籣���� 2 �жϼ��ٽ籣��

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
 * Function	: LED��ʼ��
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
 * Function	: ����IO����ӿ�
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
 * Function	: ����IO�������ʽ
 * Input	: uint8_t id �豸ID
			  uint8_t onCount ����ʱ��
			  uint8_t offCount �ر�ʱ��
			  uint8_t repeat �ظ�����
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
 * Function	: io ���豸����ִ�к���,�����˸���߳����ƿ�
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
		if (sLed[i].step == LED_IDLE) //��δ����
		{
			continue;
		}

		sLed[i].count--;
		if (sLed[i].count) //ʱ��δ��
		{
			continue;
		}

		if (sLed[i].step == LED_ON)
		{
			if (sLed[i].offCount) // �ص�ʱ����㣬��˸ģʽ
			{
				sLed[i].step = LED_OFF;
				sLed[i].count = sLed[i].offCount;
				set_leds(i, CLOSE_LED);
			}
			else
			{
				sLed[i].step = LED_OFF; //�ص�ʱ��Ϊ�㣬�����ý׶�
			}
		}

		if (sLed[i].step == LED_OFF && sLed[i].count == 0)
		{
			sLed[i].repeat--;
			if (sLed[i].repeat)
			{
				if (sLed[i].repeat == (LED_FOREVER - 1)) //һֱ���ģʽ
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
 * Function	: io �����ʱʱ��
 * Input	: None
 * Output	: None
 * Return	: �´� leds_output_deal() ִ�еļ��ʱ��
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

