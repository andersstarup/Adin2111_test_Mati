/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "udp_tester.h"

//#include "Boardsupport.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwIP_adin2111_app.h"
#include "lwip/timeouts.h"
#include "stm32l4xx_hal_tim.h"
/* USER CODE END Includes */

 // TIM_HandleTypeDef htim1;

//#include <netinet/in.h>
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
 static void MX_TIM1_Init(void);

int main(void)
{
  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  uint32_t       error;
  uint32_t       heartbeatCheckTime = 0;

  adin2111_DeviceStruct_t dev;
  adin2111_DeviceHandle_t hDevice = &dev;
  LwIP_ADIN2111_t myConn;
  board_t boardDetails;

  /****** System Init *****/
  error = BSP_InitSystem();
  DEBUG_RESULT("BSP_InitSystem", error, 0);

  BSP_HWReset(true);

  boardDetails.mac[0] =	0x00;
  boardDetails.mac[1] =	0xE0;
  boardDetails.mac[2] =	0x22;
  boardDetails.mac[3] =	0xFE;
  boardDetails.mac[4] =	0xDA;
  boardDetails.mac[5] =	0xC9; //0xB9 for 2nd board

  boardDetails.ip_addr[0] =   192;
  boardDetails.ip_addr[1] =   168;
  boardDetails.ip_addr[2] =   1;
  boardDetails.ip_addr[3] =   123; //123 for 2nd board

  boardDetails.net_mask[0] =  255;
  boardDetails.net_mask[1] =  255;
  boardDetails.net_mask[2] =  255;
  boardDetails.net_mask[3] =  0;

  boardDetails.gateway[0] =   192;
  boardDetails.gateway[1] =   168;
  boardDetails.gateway[2] =   1;
  boardDetails.gateway[3] =   1;

  boardDetails.ip_addr_fixed = IP_FIXED; //IP_DYNAMIC;

  error = discoveradin2111(&hDevice);
  DEBUG_RESULT("Failed to access ADIN2111", error, 0);

  LwIP_StructInit(&myConn, &hDevice, boardDetails.mac);
  LwIP_Init(&myConn, &boardDetails);
  LwIP_ADIN2111LinkInput(&myConn.netif);
  BSP_delayMs(500);

  netif_set_link_up(&myConn.netif);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  tftp_init();

  MX_TIM1_Init();
  HAL_TIM_Base_Start(&htim1);



  while (1)
  {
	  //HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
	  //MX_Led_Toggle(GPIOG, GPIO_PIN_14);
	  //HAL_Delay(500);
	  //HAL_Delay(500);
    /* USER CODE END WHILE */
      uint32_t now  = BSP_SysNow();
      if (now - heartbeatCheckTime >= 250)
      {
        heartbeatCheckTime = now;

        BSP_HeartBeat();

        sys_check_timeouts();

      }
      LwIP_ADIN2111LinkInput(&myConn.netif);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  __HAL_RCC_TIM1_CLK_ENABLE();

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
}



void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}




