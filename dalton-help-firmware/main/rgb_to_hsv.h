/*
 * rgb_to_hsv.h
 *
 *  Created on: 17 de mar de 2019
 *      Author: jonathan
 */

#ifndef RGB_TO_HSV_H_
#define RGB_TO_HSV_H_
#include "esp_system.h"

/*
	0 = RED
	60 = Yellow
	120 = Green
	180 = Cian
	240 = Blue
	300 = Magenta

 * */

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_colour_st;

typedef struct
{
	uint8_t h;
	uint8_t s;
	uint8_t v;
} hsv_colour_st;

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb_st;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv_st;

hsv_colour_st RgbToHsv(rgb_colour_st rgb);

rgb_colour_st HsvToRgb(hsv_colour_st hsv);

hsv_st rgb2hsv(rgb_st in);

#endif /* RGB_TO_HSV_H_ */
