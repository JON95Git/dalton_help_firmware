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

esp_err_t apds9960_test_func(apds9960_handle_t *apds9960, colour_st *color_to_diplay_st)
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

		  *color_to_diplay_st = *color_st;

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

esp_err_t lcd_init(i2c_lcd1602_info_t *lcd_info, i2c_address_t address, smbus_info_t *smbus_info){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);
	_ASSERT(smbus_info != NULL, ESP_FAIL);

	ret = smbus_init(smbus_info, I2C_NUM_0, address);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
    if (ret != ESP_OK)
    	printf("\nErro ao inicializar smbus\n");

	ret = smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
    if (ret != ESP_OK)
    	printf("\nErro ao configurar timeout\n");

	ret = i2c_lcd1602_init(lcd_info, smbus_info, true);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
    if (ret != ESP_OK)
    	printf("\nErro ao inicializar display \n");

__end:
	return ret;
}

esp_err_t gpio_button_config(void (*gpio_isr_handler)(void*)){

	esp_err_t ret = ESP_OK;

	gpio_config_t io_conf;

	//interrupt of rising edge
	io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;

	//bit mask of the pins, use GPIO4/5 here
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;

	//set as input mode
	io_conf.mode = GPIO_MODE_INPUT;

	//enable pull-up mode
	io_conf.pull_up_en = 0;

	gpio_config(&io_conf);

	//change gpio intrrupt type for one pin
	gpio_set_intr_type(GPIO_INPUT_IO_1, GPIO_PIN_INTR_POSEDGE);

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

	//remove isr handler for gpio number.
	gpio_isr_handler_remove(GPIO_INPUT_IO_1);
	//hook isr handler for specific gpio pin again
	gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

//__end:
	return ret;

}


/*
esp_err_t i2c_master_init(void)
{
	esp_err_t ret = ESP_OK;

    int i2c_master_port = I2C_NUM_0;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.scl_io_num = 22;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.master.clk_speed = 100000;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_LEN,
                       I2C_MASTER_TX_BUF_LEN, 0);

    return ret;
}
*/





