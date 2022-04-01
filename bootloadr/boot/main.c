#include "boot.h"



int main()
{
	bsp_init();
	
	update_process();
	return true;
}