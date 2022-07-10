/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define KEY2_Pin		   GPIO_PIN_2
#define KEY2_GPIO_Port	   GPIOE
#define KEY1_Pin		   GPIO_PIN_3
#define KEY1_GPIO_Port	   GPIOE
#define KEY0_Pin		   GPIO_PIN_4
#define KEY0_GPIO_Port	   GPIOE
#define KEY_UP_Pin		   GPIO_PIN_0
#define KEY_UP_GPIO_Port   GPIOA
	
#define LED1_DS1_Pin	   GPIO_PIN_5
#define LED1_DS1_GPIO_Port GPIOE
#define LED0_DS0_Pin	   GPIO_PIN_5
#define LED0_DS0_GPIO_Port GPIOB
	
#define FLASH_CS_Pin	   GPIO_PIN_12
#define FLASH_CS_GPIO_Port GPIOB

#define BEEP_Pin		   GPIO_PIN_8
#define BEEP_GPIO_Port	   GPIOB

#define NRF_IRQ_Pin		   GPIO_PIN_6
#define NRF_IRQ_GPIO_Port  GPIOG
#define NRF_CS_Pin		   GPIO_PIN_7
#define NRF_CS_GPIO_Port   GPIOG
#define NRF_CE_Pin		   GPIO_PIN_8
#define NRF_CE_GPIO_Port   GPIOG

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY2_Pin GPIO_PIN_2
#define KEY2_GPIO_Port GPIOE
#define KEY1_Pin GPIO_PIN_3
#define KEY1_GPIO_Port GPIOE
#define KEY0_Pin GPIO_PIN_4
#define KEY0_GPIO_Port GPIOE
#define LED1_DS1_Pin GPIO_PIN_5
#define LED1_DS1_GPIO_Port GPIOE
#define KEY_UP_Pin GPIO_PIN_0
#define KEY_UP_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_12
#define FLASH_CS_GPIO_Port GPIOB
#define NRF_IRQ_Pin GPIO_PIN_6
#define NRF_IRQ_GPIO_Port GPIOG
#define NRF_CS_Pin GPIO_PIN_7
#define NRF_CS_GPIO_Port GPIOG
#define NRF_CE_Pin GPIO_PIN_8
#define NRF_CE_GPIO_Port GPIOG
#define LED0_DS0_Pin GPIO_PIN_5
#define LED0_DS0_GPIO_Port GPIOB
#define BEEP_Pin GPIO_PIN_8
#define BEEP_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern SD_HandleTypeDef hsd;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern PCD_HandleTypeDef hpcd_USB_FS;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_sdio;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern void mcu_init(void);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
