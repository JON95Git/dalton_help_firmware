/*
 * dalton_internal.c
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#include "dalton_internal.h"
#include "colours.h"
#include "esp_system.h"


void IRAM_ATTR gpio_isr_handler(void* arg)
{
    xTaskNotify(xTaskHandlerLCD,0x00,eNoAction);
    xTaskNotify(xTaskHandlerAPDS,0x00,eNoAction);
}

esp_err_t dalton_init_hardware(apds9960_handle_t apds9960, i2c_lcd1602_info_t *lcd_info, i2c_bus_handle_t i2c_bus, smbus_info_t *smbus_info, i2c_address_t address ){

	esp_err_t ret = ESP_OK;

	ret = _dalton_color_sensor_init(apds9960, i2c_bus);
	if (ret != ESP_OK){
		ESP_LOGE("APDS9960", "Erro ao inicializar dispositivo I2C \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = _dalton_lcd_init(lcd_info, address, smbus_info);
	if (ret != ESP_OK){
		ESP_LOGE("LCD", "Erro ao inicializar LCD \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = _dalton_button_config(gpio_isr_handler);
	if (ret != ESP_OK){
		ESP_LOGE("GPIO", "Erro ao inicializar gpio \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

__end:
	return ret;

}

esp_err_t _dalton_button_config(void (*gpio_isr_handler)(void*)){

	esp_err_t ret = ESP_OK;
	gpio_config_t io_conf;

	_ASSERT(gpio_isr_handler != NULL, ESP_FAIL);

	io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ONLY;

	ret = gpio_config(&io_conf);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = gpio_set_intr_type(GPIO_INPUT_IO_1, GPIO_PIN_INTR_POSEDGE);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = gpio_isr_handler_remove(GPIO_INPUT_IO_1);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
	_ASSERT(ret == ESP_OK, ESP_FAIL);

__end:
	return ret;

}

esp_err_t _dalton_color_sensor_init(apds9960_handle_t *apds9960, i2c_bus_handle_t *i2c_bus)
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

	ret = i2c_param_config(i2c_master_port, &conf);
	_ASSERT(ret == ESP_OK, ESP_FAIL);

	*i2c_bus = iot_i2c_bus_create(i2c_master_port, &conf);
	if (i2c_bus == NULL){
		ret = ESP_FAIL;
		ESP_LOGE("APDS9960","Erro ao criar barramento I2C \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	*apds9960 = iot_apds9960_create(*i2c_bus, APDS9960_I2C_ADDRESS);
	if (apds9960 == NULL){
		ret = ESP_FAIL;
		ESP_LOGE("APDS9960","Erro ao criar objeto APDS9960 \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = iot_apds9960_color_init(*apds9960);
	if (ret != ESP_OK){
		ESP_LOGE("APDS9960", "Erro ao inicializar sensor \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

__end:
	return ret;
}

esp_err_t _dalton_color_get_color(apds9960_handle_t *apds9960, hsv_st *hsv){

	esp_err_t ret = ESP_OK;
	uint16_t r, g, b, c;
	rgb_st rgb;

	_ASSERT(apds9960 != NULL, ESP_FAIL);

	if (iot_apds9960_color_data_ready(apds9960) == true){
		iot_apds9960_get_color_data(apds9960, &r, &g, &b, &c);
		rgb.r = r/256;
		rgb.g = g/256;
		rgb.b = b/256;
		*hsv = rgb2hsv(rgb);
	}

__end:
	return ret;

}

esp_err_t _dalton_color_test_range(hsv_st *hsv, colour_st *color){

	esp_err_t ret = ESP_OK;

	_ASSERT(hsv != NULL, ESP_FAIL);

	if ((hsv->s >= 0.30 && hsv->v >= 0.30))
	{
		if (hsv->h >= 15.0 && hsv->h <= 45.0){
			color->hex_code = Orange;
			color->name = (uint8_t*)"Laranja";
		}
		else if (hsv->h >= 46.0 && hsv->h <= 70.0){
			color->hex_code = Yellow;
			color->name = (uint8_t*)"Amarelo";
		}
		else if (hsv->h >= 71.0 && hsv->h <= 175.0){
			color->hex_code = Green;
			color->name = (uint8_t*)"Verde";
		}
		else if (hsv->h >= 176.0 && hsv->h <= 185.0){
			color->hex_code = Cyan;
			color->name = (uint8_t*)"Ciano";
		}
		else if (hsv->h >= 186.0 && hsv->h <= 260.0){
			color->hex_code = Blue;
			color->name = (uint8_t*)"Azul";
		}
		else if (hsv->h >= 261.0 && hsv->h <= 300.0){
			color->hex_code = Purple;
			color->name = (uint8_t*)"Roxo";
		}
		else if (hsv->h >= 301.0 && hsv->h <= 330.0){
			color->hex_code = Magenta;
			color->name = (uint8_t*)"Rosa";
		}
		else if (hsv->h <= 14.0 || (hsv->h <= 360.0 && hsv->h <= 330.0)){
			color->hex_code = Red;
			color->name = (uint8_t*)"Vermelho";
		}

	}else{

		if (hsv->v <= 0.25){
			color->hex_code = Black;
			color->name = (uint8_t*)"Preto";
		}
		else if (hsv->s <= 0.10 && hsv->v >= 0.95){
			color->hex_code = White;
			color->name = (uint8_t*)"Branco";
		}
		else {
			color->hex_code = Default;
			color->name = (uint8_t*)"Nao identificado";
		}
	}
__end:
	return ret;
}

esp_err_t _dalton_lcd_init(i2c_lcd1602_info_t *lcd_info, i2c_address_t address, smbus_info_t *smbus_info){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);
	_ASSERT(smbus_info != NULL, ESP_FAIL);

	ret = smbus_init(smbus_info, I2C_NUM_0, address);
	if (ret != ESP_OK){
		ESP_LOGE("LCD", "Erro ao inicializar smbus\n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);
	if (ret != ESP_OK){
		ESP_LOGE("LCD", "Erro ao configurar timeout\n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = i2c_lcd1602_init(lcd_info, smbus_info, true);
	if (ret != ESP_OK){
		ESP_LOGE("LCD", "Erro ao inicializar display \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

__end:
	return ret;
}

esp_err_t _dalton_lcd_presentation(const i2c_lcd1602_info_t *lcd_info){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);

    ret = i2c_lcd1602_move_cursor(lcd_info, 2,0);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_write_string(lcd_info, "DALTON HELP");
	_ASSERT(ret == ESP_OK, ESP_FAIL);

__end:
	return ret;

}

esp_err_t _dalton_lcd_press_button(const i2c_lcd1602_info_t *lcd_info){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);

	ret = i2c_lcd1602_move_cursor(lcd_info, 1,0);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_write_string(lcd_info, "Press. o botao");
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_move_cursor(lcd_info, 1,1);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_write_string(lcd_info, "para ler a cor");
	_ASSERT(ret == ESP_OK, ESP_FAIL);

__end:
	return ret;

}

esp_err_t _dalton_lcd_show_color(const i2c_lcd1602_info_t *lcd_info, colour_st *color_to_diplay_st){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);

	ret = i2c_lcd1602_move_cursor(lcd_info, 2,0);
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_write_string(lcd_info, "Cor: ");
	_ASSERT(ret == ESP_OK, ESP_FAIL);
	ret = i2c_lcd1602_write_string(lcd_info,  (const char *)color_to_diplay_st->name);
	_ASSERT(ret == ESP_OK, ESP_FAIL);

__end:
	return ret;

}

esp_err_t _dalton_lcd_clear(const i2c_lcd1602_info_t *lcd_info){

	esp_err_t ret = ESP_OK;

	_ASSERT(lcd_info != NULL, ESP_FAIL);

	ret = i2c_lcd1602_clear(lcd_info);
	_ASSERT(ret == ESP_OK, ESP_FAIL);

__end:
	return ret;

}