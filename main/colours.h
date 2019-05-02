/*
 * colours.h
 *
 *  Created on: 16 de mar de 2019
 *      Author: jonathan
 */

#ifndef COLOURS_H_
#define COLOURS_H_
#include "esp_system.h"

/*
 * Color					Hex Code		Decimal Code
							#RRGGBB				(R,G,B)

 	Black					#000000				(0,0,0)
 	White					#FFFFFF				(255,255,255)
 	Red						#FF0000				(255,0,0)
 	Lime					#00FF00				(0,255,0)
 	Blue					#0000FF				(0,0,255)
 	Yellow					#FFFF00				(255,255,0)
 	Cyan / Aqua				#00FFFF				(0,255,255)
 	Magenta / Fuchsia		#FF00FF				(255,0,255)
 	Silver					#C0C0C0				(192,192,192)
 	Gray					#808080				(128,128,128)
 	Maroon					#800000				(128,0,0)
 	Olive					#808000				(128,128,0)
 	Green					#008000				(0,128,0)
 	Purple					#800080				(128,0,128)
 	Teal					#008080				(0,128,128)
 	Navy					#000080				(0,0,128)
 *
 * */

typedef enum{
	Black = 0x000000,
	White = 0xFFFFFF,
	Red = 0xFF0000,
	Lime = 0x00FF00,
	Blue = 0x0000FF,
	Yellow = 0xFFFF00,
	Cyan = 0x00FFFF,
	Magenta = 0xFF00FF,
	Silver = 0xC0C0C0,
	Gray = 0x808080,
	Maroon = 0x800000,
	Olive = 0x808000,
	Green = 0x008000,
	Purple = 0x800080,
	Teal = 0x008080,
	Navy = 0x000080,
	Orange = 0xE69B00,
	Default = -1,
}colour_rgb_t;

typedef char* color_id_t;

typedef struct{
	color_id_t id;
	colour_rgb_t hex_code;
	uint8_t *name;
}colour_st;

typedef enum{
	COLOUR_ERR_SUCESS = 0,
	COLOUR_ERR_INVALID_COLOUR,
	COLOUR_ERR_BAD_ARGUMENTS,
}colour_err_t;

colour_err_t testRGBcolour(colour_rgb_t color);

colour_err_t transformRGBcolour(uint16_t r, uint16_t g,uint16_t b, colour_rgb_t *color);

#endif /* COLOURS_H_ */
