/*
 * colours.c
 *
 *  Created on: 17 de mar de 2019
 *      Author: jonathan
 */
#include "colours.h"

colour_err_t testRGBcolour(colour_rgb_t color){

	colour_err_t ret = COLOUR_ERR_SUCESS;

	switch(color){
	case Black:
		printf("Black detected!!\n");
		break;
	case White:
		printf("White detected!!\n");
		break;
	case Red:
		printf("Red detected!!\n");
		break;
	case Lime:
		printf("Lime detected!!\n");
		break;
	case Blue:
		printf("Blue detected!!\n");
		break;
	case Yellow:
		printf("Yellow detected!!\n");
		break;
	case Cyan:
		printf("Cyan detected!!\n");
		break;
	case Magenta:
		printf("Magenta detected!!\n");
		break;
	case Silver:
		printf("Silver detected!!\n");
		break;
	case Gray:
		printf("Gray detected!!\n");
		break;
	case Maroon:
		printf("Maroon detected!!\n");
		break;
	case Olive:
		printf("Olive detected!!\n");
		break;
	case Green:
		printf("Green detected!!\n");
		break;
	case Purple:
		printf("Purple detected!!\n");
		break;
	case Teal:
		printf("Teal detected!!\n");
		break;
	case Navy:
		printf("Navy detected!!\n");
		break;
	default:
		ret = COLOUR_ERR_INVALID_COLOUR;

	}
	return ret;
}


colour_err_t transformRGBcolour(uint16_t r, uint16_t g, uint16_t b, colour_rgb_t *color){

	colour_err_t ret = COLOUR_ERR_SUCESS;

	if (r < 0 || g < 0 || b < 0 ){
		ret = COLOUR_ERR_BAD_ARGUMENTS;
		goto __end;
	}

	uint8_t red = r/255;
	uint8_t green = g/255;
	uint8_t blue = b/255;

	*color = (red<<16) | (green<<8) | blue;
	printf("Colour: 0x%06X:\n", *color);

__end:
	return ret;
}













