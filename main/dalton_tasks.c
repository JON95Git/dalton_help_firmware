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

			if (counter_range >= 85){
				printf("HSV: h = %.2f , s = %.2f, v = %.2f \n", hsv.h, hsv.s, hsv.v);
				printf("counter: %i\n", counter_range);
				printf("Cor detectada: %s\n\n", color_st.name);
				*color_to_diplay_st = color_st;
			}
			counter_range = 0;
			vTaskDelay(100 / portTICK_RATE_MS);

		}
	}
	vPortFree(apds9960);
	vTaskDelete(NULL);
}

void dalton_http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT_HTTP,
                            false, true, portMAX_DELAY);
        ESP_LOGI(TAG_HTTP, "Connected to AP");

        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG_HTTP, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG_HTTP, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG_HTTP, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_HTTP, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG_HTTP, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG_HTTP, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST_HTTP, strlen(REQUEST_HTTP)) < 0) {
            ESP_LOGE(TAG_HTTP, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_HTTP, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG_HTTP, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_HTTP, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while(r > 0);

        ESP_LOGI(TAG_HTTP, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG_HTTP, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG_HTTP, "Starting again!");
    }
}
