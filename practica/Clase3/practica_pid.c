/*
*  Control Digital en Sistemas Embebidos - MSE
*
*  Práctica PID
*/

#include <stdio.h>
#include <stdint.h>

#define N_SAMPLES 1000

static float R[N_SAMPLES];
static float Y_OPEN[N_SAMPLES];
static float Y_PID[N_SAMPLES];
static float U[N_SAMPLES];

static float numz[N_SAMPLES] = {0, 0.0201, -0.0191};
static float denz[N_SAMPLES] = {1, -1.9598, 0.9608};

typedef struct {
	float Kp;
	float Ti;
	float Td;
	float h;
	float N;
	float b;
} pid_config_t;

typedef struct {
	float pastD;
	float pastY;
	float futureI;
} pid_state_t;

void pid_init(pid_config_t* config, pid_state_t* state) {
	state->futureI = 0;
	state->pastY = 0;
	state->pastD = 0;
}

float pid_control(pid_config_t* config, pid_state_t* state, float y, float r) {
	float P = 0;
	float D = 0;
	float I = 0;
	float U = 0;

	P = config->Kp * (config->b * r - y);
	D = (config->Td * state->pastD - config->N * config->Kp * config->Td * (y-state->pastY)) / (config->Td + config->N * config->h);
	I = state->futureI;

	U = P + I + D;

	state->pastD = D;
	state->pastY = y;
	state->futureI = I + config->Kp * config->h / config->Ti * (r - y);

	return U;
}

int main() {
	pid_config_t pid_config;
	pid_state_t pid_state;

	pid_config.h = 0.01;
	pid_config.Kp = 2.6;
	pid_config.Ti = 2.6 / 0.08 * pid_config.h;
	pid_config.Td = 10 * pid_config.h / 2.6;
	pid_config.b = 1;
	pid_config.N = 20;

	pid_init(&pid_config, &pid_state);

	for (int i = 0; i < N_SAMPLES; i++){
		R [i] = i < N_SAMPLES/2 ? 0.0 : 1.0;
		Y_OPEN[i] = 0.0;
		Y_PID[i] = 0.0;
		U[i] = 0.0;
	}

	printf("\n\ry_open_c = [0;0;0;" );
	for (int n = 3; n < N_SAMPLES; n++){
		Y_OPEN[n] = numz[1] * R[n-1] + numz[2] * R[n-2] - denz[1] * Y_OPEN[n-1]- denz[2] * Y_OPEN[n-2];
		printf("%f;", Y_OPEN[n]);
	}
	printf ("]\n\r");

	printf("\n\ry_pid_c = [0;0;0;" );
	for (int n = 3; n < N_SAMPLES; n++){
        // Y_PID[n] = ADC
		U[n] = pid_control(&pid_config, &pid_state, Y_PID[n-1], R[n]);
        // DAC = U[n] 
		Y_PID[n] = numz[1] * U[n-1] + numz[2] * U[n-2] - denz[1] * Y_PID[n-1] - denz[2] * Y_PID[n-2];
		printf("%f;", Y_PID[n]);
	}
	printf("]\n\r");

	return (0);
}
