#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stdbool.h>



bool get_char(uint8_t *pChar, uint32_t dwTimeOut);
void put_char(uint8_t Data);
void put_string(char *pStr);

#endif