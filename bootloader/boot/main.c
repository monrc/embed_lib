#include "boot.h"



int main(void)
{
	bsp_init();

	update_process();
	
	return true;
}