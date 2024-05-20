/*
 * app.c
 *
 *  Created on: May 19, 2024
 *      Author: Lucas
 */

#include <app_main.h>
#include "stm32f429xx.h"
#include "arm_math.h"

// Definición de las matrices
#define ROWS 3
#define COLS 3

// Matrices de prueba
float32_t A[ROWS * COLS] = { 1.0, 2.0, 3.0,
                             4.0, 5.0, 6.0,
                             7.0, 8.0, 9.0 };

float32_t B[ROWS * COLS] = { 9.0, 8.0, 7.0,
                             6.0, 5.0, 4.0,
                             3.0, 2.0, 1.0 };

float32_t C[ROWS * COLS]; // Resultado

// Estructura de las matrices
arm_matrix_instance_f32 matA;
arm_matrix_instance_f32 matB;
arm_matrix_instance_f32 matC;

/* Declaraciones */
void APP_TaskDemo(void *pvParameter);
void APP_TaskDemoMath(void *pvParameter);

retType APP_TasksCreate(void)
{
	/* Task Size is in words -> uint32_t -> 1024 bytes / 4 -> 256 words */
	if (xTaskCreate(APP_TaskDemoMath, "Task1", 256, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
	{
		return API_ERROR;
	}

	return API_OK;
}


void APP_TaskDemo(void *pvParameter)
{
	while(1)
	{
		print_debug_msg("Hello World -> Task Demo");
		vTaskDelay(500);
	}
}

void APP_TaskDemoMath(void *pvParameter)
{
    // Inicializar las estructuras de las matrices
    arm_mat_init_f32(&matA, ROWS, COLS, (float32_t *)A);
    arm_mat_init_f32(&matB, ROWS, COLS, (float32_t *)B);
    arm_mat_init_f32(&matC, ROWS, COLS, (float32_t *)C);

    for(;;)
    {
        // Multiplicar las matrices A y B
        arm_mat_mult_f32(&matA, &matB, &matC);

        // Aquí puedes añadir un punto de interrupción o algún método para ver el resultado
        // Por ejemplo, puedes enviar el resultado a través de UART para depuración

        vTaskDelay(1000); // Esperar 1 segundo
    }
}
