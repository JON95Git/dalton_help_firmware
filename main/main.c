/*
 * main.c
 *
 *  Created on: 9 de mar de 2019
 *      Author: jonathan
 */
#include "esp_system.h"
#include "dalton_internal.h"

TaskHandle_t xTaskHandlerLed = NULL;
TaskHandle_t xTaskHandlerAPDS = NULL;
TaskHandle_t xTaskHandlerLCD = NULL;

void app_main()
{
	esp_err_t ret = ESP_OK;
	apds9960_handle_t apds9960 = NULL;
	i2c_bus_handle_t i2c_bus = NULL;
	smbus_info_t *smbus_info = smbus_malloc();
	i2c_lcd1602_info_t *lcd_info = i2c_lcd1602_malloc();
	i2c_address_t address = CONFIG_LCD1602_I2C_ADDRESS;

	ret = dalton_init_hardware(&apds9960, lcd_info, &i2c_bus, smbus_info, address);
	_ASSERT(ret == ESP_OK, ESP_FAIL);

	xTaskCreate(dalton_color_task, "dalton_color_task", 1024*2, (void *)apds9960, 5, &xTaskHandlerAPDS);
	xTaskCreate(dalton_blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, &xTaskHandlerLed);
	xTaskCreate(dalton_lcd_task, "dalton_lcd_task", 1024*2, (void *)lcd_info, 5, &xTaskHandlerLCD);

__end:
	if (ret != ESP_OK){
		ESP_LOGE("DALTON ERROR", "Fechando aplicacao... \n");
		smbus_free(&smbus_info);
		i2c_lcd1602_free(&lcd_info);
		vTaskDelete(xTaskHandlerLed);
		vTaskDelete(xTaskHandlerAPDS);
		vTaskDelete(xTaskHandlerLCD);
	}else {
		ESP_LOGI("DALTON OK", "Iniciando aplicacao... \n");
		ESP_LOGI("DALTON OK", "Pronto!\n");
	}

}
