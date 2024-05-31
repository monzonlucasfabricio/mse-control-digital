/*
 * identificacion.c
 *
 *  Created on: 30 may. 2024
 *      Author: Lucas
 */


/* Includes ------------------------------------------------------------------*/
#include "app_main.h"
#include <stdio.h>
#include <math.h>


/* Definitions for the task */
#define DATA_SIZE 100

// Noise signal limits
#define DAC_REFERENCE_VALUE_HIGH   2667  // 4095 = 3.3V, 2667 = 2.15V
#define DAC_REFERENCE_VALUE_LOW    1427  // 4095 = 3.3V, 1427 = 1.15V
#define getVoltsSampleFrom(adc0Channel) 3.3*(float)ADC_Read((adc0Channel))/4095.0
#define SCALE_U 4095 / 3.3
#define TS_MS 5
/* Variables */
double u[DATA_SIZE]; // Entrada
double y[DATA_SIZE]; // Salida
double a[3], b[2];

/* Function Prototypes */
void float_print (char *name, float buffer);
void generate_prbs_signal(double *u, int size);
void least_squares(double *u, double *y, int size, double *a, double *b);
void invert_matrix(double A[5][5], double A_inv[5][5]);
void delay_us(uint32_t us);


void identificationTask(void *pvParameters) {
    static portTickType xLastWakeTime;
    generate_prbs_signal(u, DATA_SIZE);
	xLastWakeTime = xTaskGetTickCount();
    for(;;) 
    {
        for (int i = 0; i < DATA_SIZE; i++)
        {
            DAC_Write(&hdac, u[i] * SCALE_U);
            delay_us(10);
            y[i] = getVoltsSampleFrom(CH10);
            vTaskDelayUntil( &xLastWakeTime, ( TS_MS / portTICK_RATE_MS ) );
        }

        least_squares(u, y, DATA_SIZE, a, b);

        print_debug_msg("Parametros:\n");
        float_print("a0",a[0]);
        float_print("a1",a[1]);
        float_print("a2",a[2]);
        float_print("b0",b[0]);
        float_print("b1",b[1]);
    }
}

/* PRBS Signal Generation */
void generate_prbs_signal(double *u, int size) {
    uint16_t lfsr = 0xACE1u; // Estado inicial no nulo
    uint16_t bit;

    for (int i = 0; i < size; i++) {
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
        lfsr = (lfsr >> 1) | (bit << 15);
        u[i] = (lfsr & 1) ? 2.0 : 1.0;
    }
}

/* Función para invertir una matriz 5x5 (Gauss-Jordan) */
void invert_matrix(double A[5][5], double A_inv[5][5]) {
    int i, j, k;
    static double ratio, a;

    // Inicializar A_inv como matriz identidad
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            A_inv[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Aplicar Gauss-Jordan
    for (i = 0; i < 5; i++) {
        a = A[i][i];
        for (j = 0; j < 5; j++) {
            A[i][j] /= a;
            A_inv[i][j] /= a;
        }
        for (k = 0; k < 5; k++) {
            if (k != i) {
                ratio = A[k][i];
                for (j = 0; j < 5; j++) {
                    A[k][j] -= ratio * A[i][j];
                    A_inv[k][j] -= ratio * A_inv[i][j];
                }
            }
        }
    }
}

/* Función para resolver el sistema de ecuaciones utilizando cuadrados mínimos */
void least_squares(double *u, double *y, int size, double *a, double *b) {
    static double X[DATA_SIZE][5]; // Matriz de diseño
    static double Y[DATA_SIZE];    // Vector de salida
    static double Xt[5][DATA_SIZE]; // Transpuesta de X
    static double XtX[5][5];       // Xt * X
    static double XtY[5];          // Xt * Y
    static double invXtX[5][5];    // Inversa de XtX

    // Llenar la matriz de diseño y el vector de salida
    for (int i = 3; i < size; i++) {
        X[i][0] = y[i - 1];
        X[i][1] = y[i - 2];
        X[i][2] = y[i - 3];
        X[i][3] = u[i];
        X[i][4] = u[i - 1];
        Y[i] = y[i];
    }

    // Calcular Xt
    for (int i = 0; i < 5; i++) {
        for (int j = 3; j < size; j++) {
            Xt[i][j] = X[j][i];
        }
    }

    // Calcular XtX
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            XtX[i][j] = 0;
            for (int k = 3; k < size; k++) {
                XtX[i][j] += Xt[i][k] * X[k][j];
            }
        }
    }

    // Calcular XtY
    for (int i = 0; i < 5; i++) {
        XtY[i] = 0;
        for (int k = 3; k < size; k++) {
            XtY[i] += Xt[i][k] * Y[k];
        }
    }

    // Invertir XtX
    invert_matrix(XtX, invXtX);

    // Resolver para los coeficientes a y b
    double result[5];
    for (int i = 0; i < 5; i++) {
        result[i] = 0;
        for (int j = 0; j < 5; j++) {
            result[i] += invXtX[i][j] * XtY[j];
        }
    }

    a[0] = result[0];
    a[1] = result[1];
    a[2] = result[2];
    b[0] = result[3];
    b[1] = result[4];
}


void float_print (char *name, float buffer)
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

void delay_us(uint32_t us) {
    // Calcular el número de iteraciones necesarias
    uint32_t iterations = (180 * us) / 10; // 180 MHz clock, 10 cycles per iteration

    for (uint32_t i = 0; i < iterations; ++i) {
        // Instrucción vacía para el delay
        __asm__("nop");
    }
}

