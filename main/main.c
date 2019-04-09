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

void apds9960_task(void *pvParameter)
{
    while(1) {
    	apds9960_test_func(&apds9960, color_to_diplay_st);
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

void lcd1602_task(void *pvParameter)
{

    //i2c_lcd1602_set_cursor(lcd_info, true);
    i2c_lcd1602_move_cursor(lcd_info, 2,0);
	i2c_lcd1602_write_string(lcd_info, "Dalton Help");
	vTaskDelay(3000 / portTICK_RATE_MS);
	i2c_lcd1602_clear(lcd_info);
	i2c_lcd1602_move_cursor(lcd_info, 2,0);
	i2c_lcd1602_write_string(lcd_info, "Press button ");
	i2c_lcd1602_move_cursor(lcd_info, 3,1);
	i2c_lcd1602_write_string(lcd_info, "to init");
	vTaskDelay(3000 / portTICK_RATE_MS);

	for(;;){

		/* Block indefinitely (without a timeout, so no need to check the function's
        return value) to wait for a notification.

        Bits in this RTOS task's notification value are set by the notifying
        tasks and interrupts to indicate which events have occurred. */

		if (pdTRUE == xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
				ULONG_MAX, /* Reset the notification value to 0 on exit. */
				0x00, /* Notified value pass out in ulNotifiedValue. */
				portMAX_DELAY ))  /* Block indefinitely. */
		{
			i2c_lcd1602_clear(lcd_info);
			i2c_lcd1602_move_cursor(lcd_info, 2,0);
			i2c_lcd1602_write_string(lcd_info, "Color: ");
			i2c_lcd1602_write_string(lcd_info,  (const char *)color_to_diplay_st->name);
			vTaskDelay(3000 / portTICK_RATE_MS);
       }

		i2c_lcd1602_clear(lcd_info);
		i2c_lcd1602_move_cursor(lcd_info, 2,0);
		i2c_lcd1602_write_string(lcd_info, "Press button ");
		i2c_lcd1602_move_cursor(lcd_info, 3,1);
		i2c_lcd1602_write_string(lcd_info, "again pls");
    }
    vTaskDelete(NULL);

}
void IRAM_ATTR gpio_isr_handler(void* arg)
{
    xTaskNotify(xTaskHandlerLCD,0x00,eNoAction);
}
void gpio_task(void *pvParameter)
{

    for(;;) {


        }

}

void app_main()
{
	esp_err_t ret = ESP_OK;

	lcd_info = i2c_lcd1602_malloc();
	smbus_info = smbus_malloc();
	color_to_diplay_st = (colour_st*)pvPortMalloc(sizeof(colour_st));

	ret = i2c_sensor_apds9960_init(&apds9960, &i2c_bus);
	if (ret != ESP_OK)
		printf("\nErro ao inicializar dispositivo I2C \n");

	ret = iot_apds9960_color_init(apds9960);
	if (ret != ESP_OK)
		printf("\nErro ao inicializar sensor \n");

	ret = lcd_init(lcd_info, address, smbus_info);
	if (ret != ESP_OK){
		printf("\nErro ao inicializar LCD \n");
		printf("\nret = %i \n",ret);
	}

	ret = gpio_button_config(gpio_isr_handler);
	if (ret != ESP_OK){
		printf("\nErro ao inicializar gpio \n");
	}


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

	xTaskCreate(lcd1602_task,
			"lcd1602_task",
			4096,
			NULL,
			5,
			&xTaskHandlerLCD);

	xTaskCreate(gpio_task,
			"gpio_task_example",
			2048,
			NULL,
			10,
			&xTaskHandlerGpio);

	while (1){

		//Nunca chega aqui...
	}


}





