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
#define SCALE_U         4095/3.3           // 4095 / 3.3     Voltage to 12-bit DAC

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

#define NUM_SIZE 3
#define DEN_SIZE 3

// Coeficientes del numerador en Q15

#define NUM0 1.0f
#define NUM1 -1.55892256f
#define NUM2 0.62932468f

// Coeficientes del denominador en Q15
#define DEN0 1.0f
#define DEN1 -1.45598237f
#define DEN2 0.52638449f

/*=====[Prototypes (declarations) of private functions]======================*/
static float pidRecurrenceFunction(float input);
void pid_print (char *name, float buffer);

/*=====[Implementations of public functions]=================================*/

// Para calcular el tiempo que tarda el algoritmo y establecer un h minimo
//#define COUNT_CYCLES
#define TASK_PERIOD_MS 5
// Task implementation
void pidControlTask( void* taskParmPtr )
{
   // Controller signals
   float r = 0.0f; // Measure of reference r[k]
   float y = 0.0f; // Measure of system output y[k]
   float e = 0.0f;
   float u = 0.0f; // Calculated controller's output u[k]

   // h no puede ser menor ni al tiempo del algoritmo, y con va a ser un
   // multiplo del periodo de tick del RTOS
   uint32_t h_ms = 5; // Task periodicity (sample rate)
   float h_s = ((float)h_ms)/1000.0f;

   // Enable ADC/DAC
   ADC_Init( ADC_ENABLE );
   DAC_Init( DAC_ENABLE );

   // PID Controller structure (like an object instance)
   PIDController_t PsPIDController;

   // PID Controller Initialization
   pidInit( &PsPIDController,
            3.52f,                  // Kp
            135.f / h_s,        	// Ki
            0.03528f * h_s,        	// Kd
            h_s,                   // h en [s]
            20.0f,                 // N
            1.0f,                  // b
            0.0f,                  // u_min
            3.3f                   // u_max
          );


   TickType_t xLastWakeTime;
   // Convertir el período a ticks
   const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_MS);

   // Inicializar xLastWakeTime con el tiempo actual
   xLastWakeTime = xTaskGetTickCount();

//   portTickType xPeriodicity =  h_ms / portTICK_RATE_MS;
//   portTickType xLastWakeTime = xTaskGetTickCount();
   while(true) {

      uint16_t tmp_y = ADC_Read( CH10 );
      uint16_t tmp_r = ADC_Read( CH13 );

      y = tmp_y * SCALE_Y;
      r = tmp_r * SCALE_R;
      e = 2*r - y;

      print_debug_msg("(ADC) y : %d\n",tmp_y);
      print_debug_msg("(ADC) r : %d\n",tmp_r);

      pid_print("r",r);
      pid_print("y",y);
      pid_print("e",e);

      u = pidRecurrenceFunction(e) * SCALE_U;

      pid_print("u",u);

      DAC_Write(&hdac,u);

/* Descomentar esta linea si quiero enviar la entrada directamente al sistema sin pasar por el PID */
      //DAC_Write(&hdac,tmp_r);

      vTaskDelayUntil(&xLastWakeTime, xPeriod);
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

static float pidRecurrenceFunction(float input) {

    // Buffers para mantener el estado
    static float input_buffer[NUM_SIZE] = {[0 ... NUM_SIZE - 1] = 0};
    static float output_buffer[DEN_SIZE - 1] = {[0 ... DEN_SIZE - 2] = 0};

    // Desplazar valores en el buffer de entrada
    for (int i = NUM_SIZE - 1; i > 0; --i) {
        input_buffer[i] = input_buffer[i - 1];
    }
    input_buffer[0] = input;

    // Calcular la parte del numerador
    float output = 0;
    output += (NUM0 * input_buffer[0]);
    output += (NUM1 * input_buffer[1]);
    output += (NUM2 * input_buffer[2]);

    // Calcular la parte del denominador
    output -= (DEN1 * output_buffer[0]);
    output -= (DEN2 * output_buffer[1]);

    // Desplazar valores en el buffer de salida
    for (int i = DEN_SIZE - 2; i > 0; --i) {
        output_buffer[i] = output_buffer[i - 1];
    }
    output_buffer[0] = output;

    return output;
}


void pid_print (char *name, float buffer)
{
	int32_t integer, fraction;

	print_debug_msg("%s : ",name);
	// Casteo de float a int
	integer = (int)buffer;
	fraction = (int)(((buffer - (float)integer)) * 1000);
	if (fraction<0)
	{
		fraction = (-1)*fraction;
		if (integer==0)
		{
			print_debug_msg("-%d.%03d\t", integer, fraction);
		}
		else
		{
			print_debug_msg("%d.%03d\t", integer, fraction);
		}
	}
	else
	{
		print_debug_msg("%d.%03d\t", integer, fraction);
	}
	print_debug_msg("]\t");
	print_debug_msg("\n");
}


/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
