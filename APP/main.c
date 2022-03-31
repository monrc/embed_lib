
#include "main.h"

extern void mcu_init(void);

int main(void)
{
	mcu_init();

	while (1)
	{
		HAL_Delay(500);
	}
	return 1;
}