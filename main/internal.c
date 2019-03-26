/*
 * internal.c
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#include "internal.h"
#include "colours.h"
#include "esp_system.h"

esp_err_t i2c_sensor_apds9960_init(apds9960_handle_t *apds9960, i2c_bus_handle_t *i2c_bus)
{
	esp_err_t ret = ESP_OK;

	_ASSERT(apds9960 != NULL, ESP_FAIL);
	_ASSERT(i2c_bus != NULL, ESP_FAIL);

	i2c_port_t i2c_master_port = APDS9960_I2C_MASTER_NUM;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = APDS9960_I2C_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = APDS9960_I2C_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = APDS9960_I2C_MASTER_FREQ_HZ;

	i2c_param_config(i2c_master_port, &conf);

	*i2c_bus = iot_i2c_bus_create(i2c_master_port, &conf);
	if (i2c_bus == NULL){
		printf("\nErro ao criar dispositivo I2C \n");
	}

	*apds9960 = iot_apds9960_create(*i2c_bus, APDS9960_I2C_ADDRESS);
	if (apds9960 == NULL){
		printf("\nErro ao criar objeto APDS9960 \n");
	}else
		printf("\nObjeto APDS9960: %x \n", (unsigned int)*apds9960);
__end:
	return ret;
}

esp_err_t apds9960_test_func(apds9960_handle_t *apds9960)
{

	esp_err_t ret = ESP_OK;

	_ASSERT(apds9960 != NULL, ESP_FAIL);

	uint16_t *r = (uint16_t*)pvPortMalloc(sizeof(uint16_t));
	uint16_t *g = (uint16_t*)pvPortMalloc(sizeof(uint16_t));
	uint16_t *b = (uint16_t*)pvPortMalloc(sizeof(uint16_t));
	uint16_t *c = (uint16_t*)pvPortMalloc(sizeof(uint16_t));

	rgb_st *rgb1 = (rgb_st*)pvPortMalloc(sizeof(rgb_st));
	hsv_st *hsv1 = (hsv_st*)pvPortMalloc(sizeof(hsv_st));
	colour_st *color_st = (colour_st*)pvPortMalloc(sizeof(colour_st));

    while (1)
     {

		  iot_apds9960_get_color_data(*apds9960, r, g, b, c);

		  rgb1->r = *r/256;
		  rgb1->g = *g/256;
		  rgb1->b = *b/256;

		  printf("RGB: r = %f,  g = %f,  b = %f  c = %i\n", rgb1->r, rgb1->g, rgb1->b, *c/256);

		  *hsv1 = rgb2hsv(*rgb1);
		  printf("HSV1: h = %.2f , s = %.2f, v = %.2f \n", hsv1->h, hsv1->s, hsv1->v);

		  ret = test_hsv_color_range(hsv1, color_st);
		  printf("Color detected: %s\n\n", color_st->name);

		  vTaskDelay(1000 / portTICK_RATE_MS);

    }
	iot_apds9960_delete(*apds9960, true);
	vPortFree(r);
	vPortFree(g);
	vPortFree(b);
	vPortFree(c);
	vPortFree(color_st);
__end:
  	 return ret;

}

esp_err_t test_hsv_color_range(hsv_st *hsv, colour_st *color){

	esp_err_t ret = ESP_OK;

	_ASSERT(hsv != NULL, ESP_FAIL);

	if (hsv->h <= 20.0 || (hsv->h <= 360.0 && hsv->h <= 330.0)){
		color->hex_code = Red;
		color->name = (uint8_t*)"Red";
	}
	else if (hsv->h >= 21.0 && hsv->h <= 50.0){
		color->hex_code = Orange;
		color->name = (uint8_t*)"Orange";
	}
	else if (hsv->h >= 51.0 && hsv->h <= 70.0){
		color->hex_code = Yellow;
		color->name = (uint8_t*)"Yellow";
	}
	else if (hsv->h >= 71.0 && hsv->h <= 175.0){
		color->hex_code = Green;
		color->name = (uint8_t*)"Green";
	}
	else if (hsv->h >= 176.0 && hsv->h <= 185.0){
		color->hex_code = Cyan;
		color->name = (uint8_t*)"Cyan";
	}
	else if (hsv->h >= 186.0 && hsv->h <= 260.0){
		color->hex_code = Blue;
		color->name = (uint8_t*)"Blue";
	}
	else if (hsv->h >= 261.0 && hsv->h <= 300.0){
		color->hex_code = Purple;
		color->name = (uint8_t*)"Purple";
	}
	else if (hsv->h >= 301.0 && hsv->h <= 330.0){
		color->hex_code = Magenta;
		color->name = (uint8_t*)"Magenta";
	}
	else if (hsv->v <= 0.30){
		color->hex_code = Black;
		color->name = (uint8_t*)"Black";
	}
	else if (hsv->s <= 0.2 && hsv->v >= 0.90){
		color->hex_code = White;
		color->name = (uint8_t*)"White";
	}
	else {
		color->hex_code = Default;
		color->name = (uint8_t*)"None";
	}

__end:
	return ret;
}








