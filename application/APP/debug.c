
#include "main.h"
#include "debug.h"
#include "app_layer.h"
#include "key.h"
#include "led.h"
#include "lib_port.h"


void led_test(uint32_t id, uint32_t on, uint32_t off, uint32_t repeat)
{
	start_led_task(id, on, off, repeat);
	debug("led %u on %u off %u repeat %u\r\n", id, on, off, repeat);
}


/*
 * ============================================================================
 * Function	: 软重启
 * ============================================================================
 */
void software_reset(void)
{
	__set_FAULTMASK(1); // STM32程序软件复位
	NVIC_SystemReset();
}