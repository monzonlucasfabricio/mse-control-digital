/*
*  Control Digital en Sistemas Embebidos - MSE
*
*  Práctica Pole Placement
*/

#include <stdio.h>
#include <stdint.h>

#define N_SAMPLES 1000

static float R[N_SAMPLES];
static float Y_OPEN[N_SAMPLES];
static float Y_POLE_PLACEMENT[N_SAMPLES];
static float U[N_SAMPLES];

static float NumZ[N_SAMPLES] = {0, 0.009};
static float DenZ[N_SAMPLES] = {1, -0.999};

typedef struct {
	float Ad[2][2];
	float Bd[2];
	float Cd[2];
	float K[2];
	float Ko;
} pole_placement_config_t;

// void pole_placement_init(pole_placement_config_t *config, float desired_pole) {
// 	config->K = (config->Ad - desired_pole) / (config->Bd);
// 	config->Ko = 1 / (config->Cd / (1 - config->Ad + config->Bd * config->K) * config->Bd);
// }

float pole_placement_control(pole_placement_config_t *config, float state[2], float reference) {
	return (config->Ko * reference) - (config->K[0] * state[0] + config->K[1] * state[1]);
}


int main() {
	pole_placement_config_t pole_placement_config;

	pole_placement_config.Ad[0][0] = 1.21436117;
	pole_placement_config.Ad[0][1] = -0.31140322;
	pole_placement_config.Ad[1][0] = 1.0;
	pole_placement_config.Ad[1][1] = 0;

	pole_placement_config.Bd[0] = 1.0;
	pole_placement_config.Bd[1] = 0;

	pole_placement_config.Cd[0] = 0.05779782;
	pole_placement_config.Cd[1] = 0.03924424;

	pole_placement_config.K[0] = 0.41436117;
	pole_placement_config.K[1] = -0.14140322;

	pole_placement_config.Ko = 3.81277;

	for (int i = 0; i < N_SAMPLES; i++){
		R [i] = i < N_SAMPLES/2 ? 0.0 : 1.0;
		Y_OPEN[i] = 0.0;
		Y_POLE_PLACEMENT[i] = 0.0;
		U[i] = 0.0;
	}

	printf("\n\ry_open_c = [0;" );
	for (int n = 1; n < N_SAMPLES; n++){
		Y_OPEN[n] = NumZ[1] * R[n-1] - DenZ[1] * Y_OPEN[n-1];
		printf("%f;", Y_OPEN[n]);
	}
	printf ("];\n\r");

	while(true)
	{
		print_debug_msg("\n\ry_pole_placement_c = [0;" );
		// X[n] = ADC
		U[n] = pole_placement_control(&pole_placement_config, Y_POLE_PLACEMENT[n-1], R[n]);
		// DAC = U[n] 
		Y_POLE_PLACEMENT[n] = NumZ[1] * U[n-1] - DenZ[1] * Y_POLE_PLACEMENT[n-1];
		print_debug_msg("%f;", Y_POLE_PLACEMENT[n]);
		print_debug_msg("];\n\r");
	}
	return (0);
}
