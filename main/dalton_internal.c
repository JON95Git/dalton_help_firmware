/*
 * dalton_internal.c
 *
 *  Created on: 23 de mar de 2019
 *      Author: jonathan
 */

#include "dalton_internal.h"
#include "colours.h"
#include "esp_system.h"

const char *TAG_WIFI = "WIFI";
const int CONNECTED_BIT_WIFI = BIT0;

const char *TAG_HTTP_CLIENT = "HTTP_CLIENT";
volatile bool flagButton;


void gpio_init(void)
{
    gpio_config_t cfg;
    cfg.pin_bit_mask = BIT23;
    cfg.intr_type = 0;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_down_en = 0;
    cfg.pull_up_en = 0;

    gpio_config(&cfg);
    gpio_set_level(23, 0);
}

void IRAM_ATTR gpio_isr_handler(void* arg)
{
	if (flagButton == 0){
		xTaskNotify(xTaskHandlerLCD,0x00,eNoAction);
		flagButton = 1;
	}

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

	ret = _dalton_initialize_wifi();
	if (ret != ESP_OK){
		ESP_LOGE("WI-FI", "Erro ao inicializar Wi-fi \n");
		_ASSERT(ret == ESP_OK, ESP_FAIL);
	}

	gpio_init();

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
			color->id = "3";
			color->hex_code = Orange;
			color->name = (uint8_t*)"Laranja";
		}
		else if (hsv->h >= 46.0 && hsv->h <= 70.0){
			color->id = "2";
			color->hex_code = Yellow;
			color->name = (uint8_t*)"Amarelo";
		}
		else if (hsv->h >= 71.0 && hsv->h <= 175.0){
			color->id = "6";
			color->hex_code = Green;
			color->name = (uint8_t*)"Verde";
		}
		else if (hsv->h >= 176.0 && hsv->h <= 185.0){
			color->id = "";
			color->hex_code = Cyan;
			color->name = (uint8_t*)"Ciano";
		}
		else if (hsv->h >= 186.0 && hsv->h <= 260.0){
			color->id = "5";
			color->hex_code = Blue;
			color->name = (uint8_t*)"Azul";
		}
		else if (hsv->h >= 261.0 && hsv->h <= 300.0){
			color->id = "";
			color->hex_code = Purple;
			color->name = (uint8_t*)"Roxo";
		}
		else if (hsv->h >= 301.0 && hsv->h <= 330.0){
			color->id = "";
			color->hex_code = Magenta;
			color->name = (uint8_t*)"Rosa";
		}
		else if (hsv->h <= 14.0 || (hsv->h <= 360.0 && hsv->h >= 330.0)){
			color->id = "4";
			color->hex_code = Red;
			color->name = (uint8_t*)"Vermelho";
		}

	}else{

		if (hsv->v <= 0.25){
			color->id = "0";
			color->hex_code = Black;
			color->name = (uint8_t*)"Preto";
		}
		else if (hsv->s <= 0.10 && hsv->v >= 0.95){
			color->id = "1";
			color->hex_code = White;
			color->name = (uint8_t*)"Branco";
		}
		else {
			color->id = "";
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

esp_err_t _dalton_wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT_WIFI);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT_WIFI);
        break;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t _dalton_initialize_wifi(void)
{
	esp_err_t ret = ESP_OK;

	ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(_dalton_wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG_WIFI, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    return ret;
}

void _dalton_wifi_wait_connected(void)
{
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT_WIFI, false, true, portMAX_DELAY);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG_HTTP_CLIENT, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}


void _dalton_http_post(colour_st *color_to_diplay_st)
{
	esp_err_t err = ESP_OK;

    esp_http_client_config_t config = {
        .url = "http://192.168.43.37:5001/api/data",
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST
    char *post_data = color_to_diplay_st->id;
    char url[] = "http://192.168.43.37:5001/api/data/post/";
    strcat (url, post_data);

    ESP_LOGI(TAG_HTTP_CLIENT, "HTTP POST data = %c", *post_data);
    esp_http_client_set_url(client, url);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    err = esp_http_client_perform(client);

    if (err == ESP_OK) {
    	ESP_LOGI(TAG_HTTP_CLIENT, "HTTP POST Status = %d, content_length = %d",
    			esp_http_client_get_status_code(client),
				esp_http_client_get_content_length(client));
    } else {
    	ESP_LOGE(TAG_HTTP_CLIENT, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
