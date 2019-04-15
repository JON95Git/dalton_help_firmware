/*
 * main.c
 *
 *  Created on: 9 de mar de 2019
 *      Author: jonathan
 */
#include "internal.h"
#include "esp_system.h"

i2c_bus_handle_t i2c_bus = NULL;
apds9960_handle_t apds9960 = NULL;

TaskHandle_t xTaskHandlerLed = NULL;
TaskHandle_t xTaskHandlerAPDS = NULL;
TaskHandle_t xTaskHandlerLCD = NULL;
TaskHandle_t xTaskHandlerGpio = NULL;

i2c_lcd1602_info_t *lcd_info = NULL;
smbus_info_t *smbus_info = NULL;
i2c_address_t address = CONFIG_LCD1602_I2C_ADDRESS;
colour_st *color_to_diplay_st = NULL;

void dalton_rgb_task(void *pvParameter)
{
	while(1){
		if (pdTRUE == xTaskNotifyWait(0x00, 0x00, NULL, portMAX_DELAY))
		{
			_dalton_apds9960_test_func(&apds9960, color_to_diplay_st);
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	}
	vTaskDelete(NULL);
}

void dalton_blink_task(void *pvParameter)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void dalton_lcd_task(void *pvParameter)
{
    i2c_lcd1602_move_cursor(lcd_info, 2,0);
	i2c_lcd1602_write_string(lcd_info, "DALTON HELP");
	vTaskDelay(3000 / portTICK_RATE_MS);
	i2c_lcd1602_clear(lcd_info);
	i2c_lcd1602_move_cursor(lcd_info, 1,0);
	i2c_lcd1602_write_string(lcd_info, "Press. o botao");
	i2c_lcd1602_move_cursor(lcd_info, 1,1);
	i2c_lcd1602_write_string(lcd_info, "para ler a cor");

	while(1){
		if (pdTRUE == xTaskNotifyWait(0x00, ULONG_MAX, 0x00, portMAX_DELAY))
		{
			i2c_lcd1602_clear(lcd_info);
			i2c_lcd1602_move_cursor(lcd_info, 2,0);
			i2c_lcd1602_write_string(lcd_info, "Cor: ");
			i2c_lcd1602_write_string(lcd_info,  (const char *)color_to_diplay_st->name);
			vTaskDelay(2000 / portTICK_RATE_MS);
			i2c_lcd1602_clear(lcd_info);
			i2c_lcd1602_move_cursor(lcd_info, 1,0);
			i2c_lcd1602_write_string(lcd_info, "Press. o botao");
			i2c_lcd1602_move_cursor(lcd_info, 1,1);
			i2c_lcd1602_write_string(lcd_info, "para ler a cor");
       }

    }
    vTaskDelete(NULL);

}

void IRAM_ATTR gpio_isr_handler(void* arg)
{
    xTaskNotify(xTaskHandlerLCD,0x00,eNoAction);
    xTaskNotify(xTaskHandlerAPDS,0x00,eNoAction);
}

void gpio_task(void *pvParameter)
{

	while(1) {


        }

}

void app_main()
{
	esp_err_t ret = ESP_OK;

	lcd_info = i2c_lcd1602_malloc();
	smbus_info = smbus_malloc();
	color_to_diplay_st = (colour_st*)pvPortMalloc(sizeof(colour_st));


	ret = _dalton_i2c_sensor_apds9960_init(&apds9960, &i2c_bus);
	if (ret != ESP_OK){
		ESP_LOGE("APDS9960", "Erro ao inicializar dispositivo I2C \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = iot_apds9960_color_init(apds9960);
	if (ret != ESP_OK){
		ESP_LOGE("APDS9960", "Erro ao inicializar sensor \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = _dalton_lcd_init(lcd_info, address, smbus_info);
	if (ret != ESP_OK){
		ESP_LOGE("LCD", "Erro ao inicializar LCD \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	ret = _dalton_gpio_button_config(gpio_isr_handler);
	if (ret != ESP_OK){
		ESP_LOGE("GPIO", "Erro ao inicializar gpio \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	xTaskCreate(dalton_rgb_task, "dalton_rgb_task", 1024*2, NULL, 5, &xTaskHandlerAPDS);
	xTaskCreate(dalton_blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, &xTaskHandlerLed);
	xTaskCreate(dalton_lcd_task, "dalton_lcd_task", 1024*2, NULL, 5, &xTaskHandlerLCD);

	while(1){

		//Nunca chega aqui...
	}

__end:
	ESP_LOGE("DALTON ERROR", "Fechando aplicacao... \n");
	vTaskDelete(xTaskHandlerLed);
	vTaskDelete(xTaskHandlerAPDS);
	vTaskDelete(xTaskHandlerLCD);
	vTaskDelete(xTaskHandlerGpio);

}





