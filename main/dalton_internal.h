/*
 * dalton_internal.h
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#ifndef DALTON_INTERNAL_H_
#define DALTON_INTERNAL_H_
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "iot_i2c_bus.h"
#include "iot_apds9960.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "colours.h"
#include "rgb_to_hsv.h"
#include "i2c-lcd1602.h"
#include "smbus.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_http_client.h"

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

#define CONFIG_WIFI_SSID "RedmiJon"
#define CONFIG_WIFI_PASSWORD "00000000"
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD
extern const char *TAG_WIFI;
extern const int CONNECTED_BIT_WIFI;
EventGroupHandle_t wifi_event_group;

//---------Example HTTP GET by simple socket------------------------------------------------------------
/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "google.com"
#define WEB_PORT 80
#define WEB_URL "http://google.com/"
extern const char *TAG_HTTP;
extern const char *REQUEST_HTTP;

//---------Example HTTP CLIENT--------------------------------------------------------------------------
#define MAX_HTTP_RECV_BUFFER 512
extern const char *TAG_HTTP_CLIENT;

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const char howsmyssl_com_root_cert_pem_start[];
extern const char howsmyssl_com_root_cert_pem_end[];


#define _ASSERT(expr, error) \
	do { \
		if (!(expr)) { \
			ret = error; \
			goto __end; \
		} \
	} while (0)

extern TaskHandle_t xTaskHandlerLed;
extern TaskHandle_t xTaskHandlerAPDS;
extern TaskHandle_t xTaskHandlerLCD;
extern TaskHandle_t xTaskHandlerHttp;

void dalton_color_task(void *pvParameter);
void dalton_blink_task(void *pvParameter);
void dalton_lcd_task(void *pvParameter);
void dalton_http_test_task(void *pvParameters);

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

esp_err_t _dalton_initialize_wifi(void);
void _dalton_wifi_wait_connected(void);

void _dalton_http_post(colour_st *color_to_diplay_st);


#endif /* DALTON_INTERNAL_H_ */
