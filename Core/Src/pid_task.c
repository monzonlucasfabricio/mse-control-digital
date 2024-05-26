/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/10/8
 * Version: 1.1.0
 *
 * Changelog:
 * Version 1.1.0, 2019/10/8, Santiago Germino <sgermino@retro-ciaa.com>:
 *    1) Name change for SCALE_OUT(Y), SCALE_REF(R), SCALE_PID_OUT(U).
 *    2) Proper values for SCALE_Y, SCALE_R and SCALE_U.
 *    3) Proper scaling of Ki and Kd by h_s.
 *    4) Added calls to enable and initialize ADC & DAC.
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/

#include "pid_task.h"
 
/*=====[Inclusions of private function dependencies]=========================*/

#include "app_main.h"
#include "pid_controller.h"

/*=====[Definition macros of private constants]==============================*/

#define SCALE_Y         0.0008058608    // 3.3 / 4095     12-bit ADC to Voltage
#define SCALE_R         0.0008058608    // 3.3 / 4095     12-bit ADC to Voltage
#define SCALE_U         1240.9          // 4095 / 3.3     Voltage to 12-bit DAC

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/

// Para calcular el tiempo que tarda el algoritmo y establecer un h minimo
//#define COUNT_CYCLES

// Task implementation
void pidControlTask( void* taskParmPtr )
{
   // Controller signals
   float r = 0.0f; // Measure of reference r[k]
   float y = 0.0f; // Measure of system output y[k]
   float u = 0.0f; // Calculated controller's output u[k]

   // h no puede ser menor ni al tiempo del algoritmo, y con va a ser un
   // multiplo del periodo de tick del RTOS
   uint32_t h_ms = 4; // Task periodicity (sample rate)
   float h_s = ((float)h_ms)/1000.0f;

   // Enable ADC/DAC
   ADC_Init( ADC_ENABLE );
   DAC_Init( DAC_ENABLE );

   // PID Controller structure (like an object instance)
   PIDController_t PsPIDController;

   // PID Controller Initialization
   pidInit( &PsPIDController,
            2.8f,                  // Kp
            1.2f / h_s,            // Ki
            0.1f * h_s,            // Kd
            h_s,                   // h en [s]
            20.0f,                 // N
            1.0f,                  // b
            0.0f,                  // u_min
            3.3f                   // u_max
          );

   // Peridodic task each h_ms
   portTickType xPeriodicity =  h_ms / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();

   #ifdef COUNT_CYCLES
      // Configura el contador de ciclos con el clock de la EDU-CIAA NXP
      cyclesCounterConfig(EDU_CIAA_NXP_CLOCK_SPEED);
      volatile uint32_t cyclesElapsed = 0;
   #endif

   while(true) {

      #ifdef COUNT_CYCLES
         // Resetea el contador de ciclos
         cyclesCounterReset();
      #endif

      // Leer salida y[k] y refererencia r[k]
      y = ADC_Read( 0 ) * SCALE_Y;
      r = ADC_Read( 1 ) * SCALE_R;

      // Calculate PID controller output u[k]
      u = pidCalculateControllerOutput( &PsPIDController, y, r ) * SCALE_U;

      // Actualizar la salida u[k]
      DAC_Write(&hdac,u);

      // Update PID controller for next iteration
      pidUpdateController( &PsPIDController, y, r );

      #ifdef COUNT_CYCLES
         // Leer conteco actual de ciclos
         cyclesElapsed = DWT_CYCCNT;
         volatile float us = cyclesCounterToUs(cyclesElapsed);
      #endif

      // Send the task to the locked state during xPeriodicity
      // (periodical delay)
      vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
}


void PID_TaskDemo(void* pvParameter)
{
	ADC_Init( ADC_ENABLE );
	DAC_Init( DAC_ENABLE );

	  // Controller signals
	float r = 0.0f; // Measure of reference r[k]
	float y = 0.0f; // Measure of system output y[k]
//	float u = 0.0f; // Calculated controller's output u[k]
	uint16_t tmp_y;
	uint16_t tmp_r;

	char floatbuf[50];

	while(true)
	{
		tmp_r = ADC_Read( CH3  ); // Entrada
		tmp_y = ADC_Read( CH10 ); // Salida

		y = tmp_y * SCALE_Y; // Entrada
		r = tmp_r * SCALE_R; // Salida

		print_debug_msg("Y : %d",tmp_y);
		print_debug_msg("R : %d",tmp_r);

		vTaskDelay(10);
	}

}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/