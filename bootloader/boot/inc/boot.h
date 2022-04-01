#ifndef APP_LAYER_H_
#define APP_LAYER_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>




void bsp_init(void);
void jump_to_app(void);
void update_process(void);

#endif