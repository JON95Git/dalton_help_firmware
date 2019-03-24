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


void apds9960_task(void *pvParameter)
{
    while(1) {
    	apds9960_test_func(&apds9960);
    	vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void blink_task(void *pvParameter)
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


void app_main()
{
	esp_err_t ret = ESP_OK;

	ret = i2c_sensor_apds9960_init(&apds9960, &i2c_bus);
    if (ret != ESP_OK)
    	printf("\nErro ao inicializar dispositivo I2C \n");
    ret = iot_apds9960_color_init(apds9960);
    if (ret != ESP_OK)
    	printf("\nErro ao inicializar sensor \n");
    
    xTaskCreate(apds9960_task,
    				"apds9960_task",
    				1024*2, 
    				NULL, 
    				5, 
    				&xTaskHandlerAPDS);

    xTaskCreate(blink_task, 
    				"blink_task", 
    				configMINIMAL_STACK_SIZE, 
    				NULL, 
    				5, 
    				&xTaskHandlerLed);

}





