/*
 * app.c
 *
 *  Created on: May 19, 2024
 *      Author: Lucas
 */

#include <app_main.h>
#include "stm32f429xx.h"
#include "arm_math.h"
#include "ram_db.h"


/* ================= IDENTIFICACION ================= */

// ADC0 Channels for u and y
#define ADC0_CH_Y       CH3

// Noise signal limits
#define DAC_REFERENCE_VALUE_HIGH   2667  // 4095 = 3.3V, 2667 = 2.15V
#define DAC_REFERENCE_VALUE_LOW    1427  // 4095 = 3.3V, 1427 = 1.15V

// ADC_Read() returns 12 bits integer sample (uint16_t)
// sampleInVolts = (3.3 / 1023.0) * adcSample
#define getVoltsSampleFrom(adc0Channel) 3.3*(float)ADC_Read((adc0Channel))/4095.0

static StackType_t taskIdentificationStack[configMINIMAL_STACK_SIZE*15];
static StaticTask_t taskIdentificationTCB;

t_IRLSdata* tIRLS1;
t_ILSdata* tILS1;

void receiveData (float* buffer);

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
void APP_TaskTestRamdb(void *pvParameter);

retType APP_TasksCreate(void)
{
	/* Task Size is in words -> uint32_t -> 1024 bytes / 4 -> 256 words */
	if (xTaskCreate(PID_TaskDemo, "Task1", 256, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
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
        vTaskDelay(1000); // Esperar 1 segundo
    }
}


void APP_TaskTestRamdb(void *pvParameter)
{
    for(;;)
    {
        test_u8();
        vTaskDelay(1000);
    }
}


/*************************************************************************************************/
/* Functions for Digital Control																 */
/*************************************************************************************************/

retType APP_InitIdentif(void)
{
	    // ----- Initialize ADC and DAC ------------------
	    ADC_Init(ADC_ENABLE);
	    DAC_Init(DAC_ENABLE);

	    tIRLS1 = (t_IRLSdata*) pvPortMalloc (sizeof(t_IRLSdata));
	    tILS1 = (t_ILSdata*) pvPortMalloc (sizeof(t_ILSdata));

		IRLS_Init(tIRLS1, 10, receiveData);
		ILS_Init(tILS1, 50, 10, receiveData);

	    xTaskCreateStatic(
	        ILS_Task,                   // task function
	        "Identification Task",      // human-readable neame of task
	        configMINIMAL_STACK_SIZE,   // task stack size
	        (void*)tILS1,               // task parameter (cast to void*)
	        tskIDLE_PRIORITY+1,         // task priority
	        taskIdentificationStack,    // task stack (StackType_t)
	        &taskIdentificationTCB      // pointer to Task TCB (StaticTask_t)
	    );

	    // xTaskCreateStatic(
	    //     IRLS_Task,                  // task function
	    //     "Identification Task",      // human-readable neame of task
	    //     configMINIMAL_STACK_SIZE,   // task stack size
	    //     (void*)tIRLS1,              // task parameter (cast to void*)
	    //     tskIDLE_PRIORITY+1,         // task priority
	    //     taskIdentificationStack,    // task stack (StackType_t)
	    //     &taskIdentificationTCB      // pointer to Task TCB (StaticTask_t)
	    // );
	    return API_OK;
}

void receiveData (float* buffer)
{
    float Y, U;

    uint16_t dacValue = 0;

    // random = limite_inferior + rand() % (limite_superior +1 - limite_inferior);
    dacValue = DAC_REFERENCE_VALUE_LOW + rand() % (DAC_REFERENCE_VALUE_HIGH+1 - DAC_REFERENCE_VALUE_LOW);

    DAC_Write(&hdac, dacValue );

    // Need at least 2.5 us to uptate DAC.
    // delayInaccurateUs(5);
    vTaskDelay(1);

    // dacSample = (1023.0 / 3.3) * sampleInVolts
    // 1023.0 / 3.3 = 310.0
    U = (float) dacValue * 3.3 / 4095.0;
	Y = (float) getVoltsSampleFrom( ADC0_CH_Y );

	buffer[0] = U;
	buffer[1] = Y;
}
