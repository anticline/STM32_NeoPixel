/*
 * neoPixel.h
 *
 *  Created on: Oct 2, 2021
 *      Author: anticline
 */

#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

void set_RGBW(uint8_t p,uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void set_RGB(uint8_t p, uint8_t r, uint8_t g, uint8_t b);
void set_all_RGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void set_all_RGB(uint8_t r, uint8_t g, uint8_t b);
void renderPixels();

#endif /* INC_NEOPIXEL_H_ */
