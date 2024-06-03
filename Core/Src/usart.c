/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#define DEBUG_MSG_ENABLE

extern QueueHandle_t uart3RxQueue;
extern SemaphoreHandle_t xMutex;
extern uint8_t rxBuffer[12];
extern float globalVar1, globalVar2, globalVar3;

/* USER CODE END 0 */

UART_HandleTypeDef huart3;

/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN USART3_MspInit 1 */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOD, STLK_RX_Pin|STLK_TX_Pin);

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// Blocking Write 1 byte to TX FIFO
void uartWriteByte( UART_HandleTypeDef *huart, const uint8_t value)
{
	uint8_t val = value;
	HAL_UART_Transmit(huart, &val, 1, HAL_MAX_DELAY);
}

// Blocking, Send a Byte Array
void uartWriteByteArray( UART_HandleTypeDef *huart, char* byteArray, uint32_t byteArrayLen )
{
   uint32_t i = 0;
   for( i=0; i<byteArrayLen; i++ ) {
	   uartWriteByte(huart, byteArray[i]);
   }
}

/* Print formatted string to console over uart */
void print_debug_msg(char *format,...)
{
#ifdef DEBUG_MSG_ENABLE
	char str[80];

	/* Extract the argument list using VA apis */
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
//	strcat(str, "\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
#endif
}

void uartWriteString( UART_HandleTypeDef *huart, const char* str )
{
   while( *str != 0 ) {
      uartWriteByte( huart, (uint8_t)*str );
      str++;
   }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(uart3RxQueue, rxBuffer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        // Re-enable the UART receive interrupt
        HAL_UART_Receive_IT(&huart3, rxBuffer, RX_BUFFER_SIZE);
    }
}

void UART3Task(void *pvParameters)
{
    uint8_t receivedData[RX_BUFFER_SIZE];
    while (1)
    {
        if (xQueueReceive(uart3RxQueue, &receivedData, portMAX_DELAY) == pdPASS)
        {
            // Convert received data to floats
            float tempVar1, tempVar2, tempVar3;
            memcpy(&tempVar1, &receivedData[0], sizeof(float));
            memcpy(&tempVar2, &receivedData[4], sizeof(float));
            memcpy(&tempVar3, &receivedData[8], sizeof(float));

            // Take the mutex before updating the global variables
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
            {
                globalVar1 = tempVar1;
                globalVar2 = tempVar2;
                globalVar3 = tempVar3;
                xSemaphoreGive(xMutex);
            }

            // Process the received floats if needed
        }
    }
}
/* USER CODE END 1 */
