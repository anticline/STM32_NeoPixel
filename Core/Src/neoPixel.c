/*
 * neoPixel.c
 *
 *  Created on: Oct 2, 2021
 *      Author: anticline
 *
 *  A library to control NeoPixel LED's with STM32F4xx Controllers.
 *  It uses a Timer with DMA to create the needed PWM Signal
 *  for the LED's.
 *
 *  Still work in Progress!
 */

#include "stm32f4xx_hal.h"
extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim2_ch1;

#include "neoPixel.h"
/* Calculation for the HI, LO and RST values are as follows:
 *
 * Clock of the Timer is 84MHz, without prescaler we get the number of
 * counts for one Period at 800kHz by (84MHz/800kHz)-1 = 104.
 * A High Signal should be at 64%, so HI = 105 * 0,64 = 68.
 * A Low Signal should be at 34%, so LO = 105 * 0,34 = 34.
 * For Reset we need 80µs for sk6812 :
 *
 * */

#define HI (68)
#define LO (34)
#define NUM_PIXELS (3)

//#define BYTES_PER_PIXEL (4)			//sk6812
#define BYTES_PER_PIXEL (3)		//ws2812
#define RST (64)					//sk8612
//#define RST (48)					//ws2812

#define DATA_LENGTH (NUM_PIXELS * BYTES_PER_PIXEL * 8 + RST)

uint8_t RGB_data [BYTES_PER_PIXEL * NUM_PIXELS] = {0};
uint32_t PWM_data [DATA_LENGTH] = {0};

void set_RGBW(uint8_t p, uint8_t r, uint8_t g, uint8_t b, uint8_t w){

	RGB_data [(p * BYTES_PER_PIXEL)] = g;
	RGB_data [(p * BYTES_PER_PIXEL) + 1] = r;
	RGB_data [(p * BYTES_PER_PIXEL) + 2] = b;
	if (BYTES_PER_PIXEL == 4){
		RGB_data [(p * BYTES_PER_PIXEL) + 3] = w;
	}
}

void set_RGB(uint8_t p, uint8_t r, uint8_t g, uint8_t b){

	RGB_data [(p * BYTES_PER_PIXEL)] = g;
	RGB_data [(p * BYTES_PER_PIXEL) + 1] = r;
	RGB_data [(p * BYTES_PER_PIXEL) + 2] = b;
	if (BYTES_PER_PIXEL == 4){
		RGB_data [(p * BYTES_PER_PIXEL) + 3] = 0;
	}
}

void set_all_RGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w){

	uint8_t p;

	for (p = 0; p < NUM_PIXELS; p++){				//going through the pixels
		RGB_data[(p * BYTES_PER_PIXEL)] = g;
		RGB_data[(p * BYTES_PER_PIXEL) + 1] = r;
		RGB_data[(p * BYTES_PER_PIXEL) + 2] = b;
		if (BYTES_PER_PIXEL == 4){
			RGB_data[(p * BYTES_PER_PIXEL) + 3] = w;
		}
	}
}

void set_all_RGB(uint8_t r, uint8_t g, uint8_t b){

	uint8_t p;

	for (p = 0; p < NUM_PIXELS; p++){				//going through the pixels
		RGB_data[(p * BYTES_PER_PIXEL)] = g;
		RGB_data[(p * BYTES_PER_PIXEL) + 1] = r;
		RGB_data[(p * BYTES_PER_PIXEL) + 2] = b;
		if (BYTES_PER_PIXEL == 4){
			RGB_data[(p * BYTES_PER_PIXEL) + 3] = 0;
		}
	}
}

void renderPixels(){
	uint8_t mask = 1U << 7;
	uint8_t i, pc, rs, gs, bs, ws;

	for (pc = 0; pc < NUM_PIXELS; pc++){
		gs = RGB_data [(pc * BYTES_PER_PIXEL)];
		rs = RGB_data [(pc * BYTES_PER_PIXEL) + 1];
		bs = RGB_data [(pc * BYTES_PER_PIXEL) + 2];
		if (BYTES_PER_PIXEL == 4){
			ws = RGB_data [(pc * BYTES_PER_PIXEL) + 3];
		}

		for (i = 0; i < 8; i++){					//r
			PWM_data [i + (pc * (BYTES_PER_PIXEL*8))] = (gs & mask) ? HI : LO;
			gs <<= 1;
		}
		for (i = 8; i < 16; i++){					//g
			PWM_data [i + (pc * (BYTES_PER_PIXEL*8))] = (rs & mask) ? HI : LO;
			rs <<= 1;
		}
		for (i = 16; i < 24; i++){					//b
			PWM_data [i + (pc * (BYTES_PER_PIXEL*8))] = (bs & mask) ? HI : LO;
			bs <<= 1;
		}
		if (BYTES_PER_PIXEL == 4){
			for (i = 24; i < 32; i++){					//w
				PWM_data [i + (pc * (BYTES_PER_PIXEL*8))] = (ws & mask) ? HI : LO;
				ws <<= 1;
			}
		}
		if (pc == (NUM_PIXELS -1)){
			for (i = (BYTES_PER_PIXEL * 8); i < (BYTES_PER_PIXEL * 8) + RST; i++){					//reset
				PWM_data [i + (pc * (BYTES_PER_PIXEL*8))] = 0;
			}
		}
	}

	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, PWM_data , DATA_LENGTH);
}


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);

}
