/*
 * dalton_tasks.c
 *
 *  Created on: 19 de abril de 2019
 *      Author: jonathan
 */

#include "colours.h"
#include "esp_system.h"
#include "dalton_internal.h"

colour_st *color_to_diplay_st = NULL;

void dalton_blink_task(void *pvParameter)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        if (pdTRUE == xTaskNotifyWait(0x00, ULONG_MAX, 0x00, portMAX_DELAY)){
            gpio_set_level(23, 1);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_set_level(23, 0);
        }

    }
	vTaskDelete(NULL);
}

void dalton_lcd_task(void *pvParameter)
{
	i2c_lcd1602_info_t *lcd_info = (i2c_lcd1602_info_t*)pvParameter;

	_dalton_lcd_presentation(lcd_info);
	vTaskDelay(3000 / portTICK_RATE_MS);
	_dalton_lcd_clear(lcd_info);
	_dalton_lcd_press_button(lcd_info);
	while(1){
		if (pdTRUE == xTaskNotifyWait(0x00, ULONG_MAX, 0x00, portMAX_DELAY))
		{
			xTaskNotify(xTaskHandlerAPDS,0x00,eNoAction);
			_dalton_lcd_clear(lcd_info);
			_dalton_lcd_show_color(lcd_info, color_to_diplay_st);
			vTaskDelay(2000 / portTICK_RATE_MS);
			_dalton_lcd_clear(lcd_info);
			_dalton_lcd_press_button(lcd_info);
		}

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	vPortFree(lcd_info);
	vTaskDelete(NULL);
}

void dalton_color_task(void *pvParameter)
{
	apds9960_handle_t apds9960 = (apds9960_handle_t)pvParameter;
	color_to_diplay_st = (colour_st*)pvPortMalloc(sizeof(colour_st));
	colour_st color_st;
	colour_rgb_t hex_code = Default;
	uint8_t counter_range = 0;
	hsv_st hsv;

	while(1){
		if (pdTRUE == xTaskNotifyWait(0x00, 0x00, NULL, portMAX_DELAY))
		{
			_dalton_color_get_color(apds9960, &hsv);
			_dalton_color_test_range(&hsv, &color_st);
			*color_to_diplay_st = color_st;
			hex_code = color_st.hex_code;

			for (int i = 0; i <= MAX_READ_HSV; i++){
				_dalton_color_get_color(apds9960, &hsv);
				_dalton_color_test_range(&hsv, &color_st);

				if (color_st.hex_code == hex_code){
					counter_range++;
				}
				hex_code = color_st.hex_code;
			}

			if (counter_range >= 185){
				printf("HSV: h = %.2f , s = %.2f, v = %.2f \n", hsv.h, hsv.s, hsv.v);
				printf("counter: %i\n", counter_range);
				printf("Cor detectada: %s\n\n", color_st.name);
				*color_to_diplay_st = color_st;
				//Notifica a task de Http somente apos uma leitura concluida...
				xTaskNotify(xTaskHandlerHttp,0x00,eNoAction);
				xTaskNotify(xTaskHandlerLed,0x00,eNoAction);
			}
			counter_range = 0;
			vTaskDelay(100 / portTICK_RATE_MS);

		}
	}
	vPortFree(apds9960);
	vTaskDelete(NULL);
}

void dalton_http_test_task(void *pvParameters)
{
	_dalton_wifi_wait_connected();
	ESP_LOGI(TAG_HTTP_CLIENT, "Connected to Acess Point...");

	while (1){
		if (pdTRUE == xTaskNotifyWait(0x00, ULONG_MAX, 0x00, portMAX_DELAY))
		{
			_dalton_wifi_wait_connected();
			ESP_LOGI(TAG_HTTP_CLIENT, "Connected to Acess Point...");
			ESP_LOGI(TAG_HTTP_CLIENT, "HTTP POST id color = %s", color_to_diplay_st->id);
			_dalton_http_post(color_to_diplay_st);
		}
	}
	ESP_LOGI(TAG_HTTP_CLIENT, "Finish http");
	vTaskDelete(NULL);
}

