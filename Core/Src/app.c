/*
 * app.c
 *
 *  Created on: May 19, 2024
 *      Author: Lucas
 */

#include "app.h"

/* Declaraciones */
void task1(void *pvParameter);

retType APP_TasksCreate(void)
{
	/* Task Size is in words -> uint32_t -> 1024 bytes / 4 -> 256 words */
	if (xTaskCreate(task1, "Task1", 256, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
	{
		return API_ERROR;
	}

	return API_OK;
}


void task1(void *pvParameter)
{
	while(1)
	{
		print_debug_msg("Hello World -> Task 1");
		vTaskDelay(1000);
	}
}
