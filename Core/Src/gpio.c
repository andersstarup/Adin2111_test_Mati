/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/**
  *  Portions Copyright (c) 2020, 2021 Analog Devices, Inc.
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "tim.h"
#include "udp_tester.h"

/* USER CODE BEGIN 0 */

/* Store the IRQ callback and user parameter locally */
static          ADI_CB gpfIntCallback = NULL;
static void     *gpIntCBParam = NULL;
int counterrot = 0;
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    ETH_GPIO_ENABLE;

    /*Configure GPIO pin Output Level */
#if defined(EVAL_ADIN1110EBZ)
    HAL_GPIO_WritePin(BSP_LED1_PORT, BSP_LED1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BSP_LED2_PORT, BSP_LED2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BSP_LED3_PORT, BSP_LED3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BSP_LED4_PORT, BSP_LED4_PIN, GPIO_PIN_SET);

    HAL_PWREx_EnableVddIO2();
#else
    HAL_GPIO_WritePin(BSP_LED1_PORT, BSP_LED1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BSP_LED2_PORT, BSP_LED2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BSP_LED3_PORT, BSP_LED3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BSP_LED4_PORT, BSP_LED4_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BSP_LED5_PORT, BSP_LED5_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

#endif

    HAL_PWREx_EnableVddIO2(); // TILFØJET DA I/O Debug pins ikke virkede

    HAL_GPIO_WritePin(ETH_SPI_SS_GPIO_Port, ETH_SPI_SS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO PIN : B9*/
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO PIN : A7*/
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* SETUP BY ANDERS
    Configure GPIO pin : PG14 DEBUG2 PIN */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /*Configure GPIO pin : PG13 DEBUG PIN*/
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    /* SETUP BY ANDERS

    Configure GPIO pin : PtPin */
    HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = ETH_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ETH_RESET_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PEPin PEPin */
    GPIO_InitStruct.Pin = BSP_LED4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED4_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BSP_LED2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED2_PORT, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin */
    GPIO_InitStruct.Pin = BSP_LED1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED1_PORT, &GPIO_InitStruct);

#if !defined(EVAL_ADIN1110EBZ)
    GPIO_InitStruct.Pin = BSP_LED5_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED5_PORT, &GPIO_InitStruct);
#endif

    GPIO_InitStruct.Pin = BSP_LED3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED3_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ETH_SPI_SS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ETH_SPI_SS_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin */
    GPIO_InitStruct.Pin = CFG0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CFG0_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CFG1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CFG1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CFG2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CFG2_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CFG3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CFG3_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = ETH_INT_N_Pin;
    HAL_GPIO_Init(ETH_INT_N_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
void MX_Led_Toggle(void)
{
	HAL_GPIO_TogglePin(BSP_LED3_PORT, BSP_LED3_PIN);
#if !defined(EVAL_ADIN1110EBZ)
	HAL_GPIO_TogglePin(BSP_LED5_PORT, BSP_LED5_PIN);
#endif
	HAL_GPIO_TogglePin(BSP_LED1_PORT, BSP_LED1_PIN);
	HAL_GPIO_TogglePin(BSP_LED4_PORT, BSP_LED4_PIN);
	HAL_GPIO_TogglePin(BSP_LED2_PORT, BSP_LED2_PIN);
}
/* USER CODE END 2 */


uint32_t HAL_INT_N_Register_Callback(ADI_CB const *pfCallback, void *const pCBParam)
{
    HAL_NVIC_SetPriority(ETH_INT_N_IRQn, 0xF, 0);

    gpfIntCallback = (ADI_CB)pfCallback;
    gpIntCBParam = pCBParam ;

    return 0;
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if (GPIO_Pin == ETH_INT_N_Pin)
  {
    if (gpfIntCallback)
    {
        (*gpfIntCallback)(gpIntCBParam, 0, NULL);
    }
  }
  if(GPIO_Pin == GPIO_PIN_14){
	HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
	if(GoZeroFlag == 1){
		HAL_TIM_Base_Stop_IT(&htim2);
		__HAL_TIM_SET_COUNTER(&htim2, 0);
		GoZeroFlag = 0;
	}
   }


}


uint32_t HAL_INT_N_DisableIRQ(void)
{
    HAL_NVIC_DisableIRQ(ETH_INT_N_IRQn);

    return 0;
}

uint32_t HAL_INT_N_EnableIRQ(void)
{
    HAL_NVIC_EnableIRQ(ETH_INT_N_IRQn);

    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
