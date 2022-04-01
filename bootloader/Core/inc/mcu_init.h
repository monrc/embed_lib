/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);


/* Private defines -----------------------------------------------------------*/
#define KEY2_Pin				GPIO_PIN_2
#define KEY2_GPIO_Port			GPIOE
#define KEY1_Pin				GPIO_PIN_3
#define KEY1_GPIO_Port			GPIOE
#define KEY0_Pin				GPIO_PIN_4
#define KEY0_GPIO_Port			GPIOE
#define LED1_Pin				GPIO_PIN_5
#define LED1_GPIO_Port			GPIOE
#define KEY_UP_Pin				GPIO_PIN_0
#define KEY_UP_GPIO_Port		GPIOA
#define LED0_Pin				GPIO_PIN_5
#define LED0_GPIO_Port			GPIOB
#define BEEP_Pin				GPIO_PIN_8
#define BEEP_GPIO_Port			GPIOB
/* USER CODE BEGIN Private defines */

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_IWDG_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_TIM6_Init(void);


void mcu_init(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
