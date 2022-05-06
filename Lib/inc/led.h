
#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

#define LED_FOREVER			0xFF
#define LED_SCAN_TIME		50 //时间基为100ms
#define TIME_TO_COUTN(time) ((time) / LED_SCAN_TIME)

typedef enum
{
	LED_GREEN,
	LED_BLUE,
	BEEPER,
} IODEVICE_t;

typedef struct
{
	uint8_t id;
	uint8_t repeat;
	uint8_t onCount;
	uint8_t offCount;
} LedMessage_t;

void led_init(void);

void set_output(IODEVICE_t id, uint8_t onCount, uint8_t offCount, uint8_t repeat);

void leds_output_deal(void);

uint32_t get_output_wait_time(void);

#endif
