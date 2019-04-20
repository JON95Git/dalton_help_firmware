/*
 * dalton_internal.h
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#ifndef DALTON_INTERNAL_H_
#define DALTON_INTERNAL_H_
#include <stdio.h>
#include "unity.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "iot_i2c_bus.h"
#include "iot_apds9960.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "colours.h"
#include "rgb_to_hsv.h"
#include "i2c-lcd1602.h"
#include "smbus.h"

#define MAX_READ_HSV 200
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_1)
#define ESP_INTR_FLAG_DEFAULT 0

#define CONFIG_BLINK_GPIO 2
#define BLINK_GPIO CONFIG_BLINK_GPIO

#define APDS9960_I2C_MASTER_SCL_IO           (gpio_num_t)22
#define APDS9960_I2C_MASTER_SDA_IO           (gpio_num_t)21
#define APDS9960_I2C_MASTER_NUM              I2C_NUM_0
#define APDS9960_I2C_MASTER_TX_BUF_DISABLE   0
#define APDS9960_I2C_MASTER_RX_BUF_DISABLE   0
#define APDS9960_I2C_MASTER_FREQ_HZ          100000
#define CONFIG_LCD1602_I2C_ADDRESS 0x3c

#define _ASSERT(expr, error) \
	do { \
		if (!(expr)) { \
			ret = error; \
			goto __end; \
		} \
	} while (0)


extern TaskHandle_t xTaskHandlerLed;
extern TaskHandle_t xTaskHandlerAPDS;
extern TaskHandle_t xTaskHandlerLCD ;

void dalton_color_task(void *pvParameter);
void dalton_blink_task(void *pvParameter);
void dalton_lcd_task(void *pvParameter);

esp_err_t dalton_init_hardware(apds9960_handle_t apds9960, i2c_lcd1602_info_t *lcd_info,
		i2c_bus_handle_t i2c_bus, smbus_info_t *smbus_info, i2c_address_t address );

esp_err_t _dalton_color_sensor_init(apds9960_handle_t *apds9960, i2c_bus_handle_t *i2c_bus);
esp_err_t _dalton_color_get_color(apds9960_handle_t *apds9960, hsv_st *hsv);
esp_err_t _dalton_color_test_range(hsv_st *hsv, colour_st *color);

esp_err_t _dalton_button_config(void (*gpio_isr_handler)(void*));

esp_err_t _dalton_lcd_init(i2c_lcd1602_info_t *lcd_info, i2c_address_t address, smbus_info_t *smbus_info);
esp_err_t _dalton_lcd_presentation(const i2c_lcd1602_info_t *lcd_info);
esp_err_t _dalton_lcd_press_button(const i2c_lcd1602_info_t *lcd_info);
esp_err_t _dalton_lcd_show_color(const i2c_lcd1602_info_t *lcd_info, colour_st *color_to_diplay_st);
esp_err_t _dalton_lcd_clear(const i2c_lcd1602_info_t *lcd_info);

#endif /* DALTON_INTERNAL_H_ */
