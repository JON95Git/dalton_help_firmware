/*
 * internal.h
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#ifndef INTERNAL_H_
#define INTERNAL_H_
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


esp_err_t apds9960_test_func(apds9960_handle_t *apds9960, colour_st *color_to_diplay_st);
esp_err_t i2c_sensor_apds9960_init(apds9960_handle_t *apds9960, i2c_bus_handle_t *i2c_bus);
esp_err_t test_hsv_color_range(hsv_st *hsv, colour_st *color);
esp_err_t lcd_init(i2c_lcd1602_info_t *lcd_info, i2c_address_t address, smbus_info_t *smbus_info);
//esp_err_t i2c_master_init(void);

#endif /* INTERNAL_H_ */
