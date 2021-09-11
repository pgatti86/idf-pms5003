#include "idf-pmsx003.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "pmsx003";

static const int pmsx_baud_rate = 9600;

static const int uart_buffer_size = 1024 * 2;

static TaskHandle_t xReadTaskHandle = NULL;

/*---------------- STATIC FUNCTIONS DEFs ----------------------------------*/
static esp_err_t pms_uart_read(pmsx003_config_t *config, uint8_t *data);

static pm_data_t decodepm_data(uint8_t *data, uint8_t start_byte);

static void pmsx_data_read_task();
/*--------------------------------------------------------------------------*/

esp_err_t idf_pmsx5003_init(pmsx003_config_t *config) {

    if (xReadTaskHandle != NULL) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "init pmsx sensor");

    uart_config_t uart_config = {
        .baud_rate = pmsx_baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    esp_err_t result = uart_param_config(config->uart_port, &uart_config);
    if (result != ESP_OK)
        return ESP_FAIL;
    
    result = uart_set_pin(config->uart_port, config->uart_tx_pin, config->uart_rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (result != ESP_OK)
        return ESP_FAIL;
    
    result = uart_driver_install(config->uart_port, uart_buffer_size, uart_buffer_size, 0, NULL, 0);
    if (result != ESP_OK)
        return ESP_FAIL;

    xTaskCreate(pmsx_data_read_task, "data read task", 2048, config, 5, &xReadTaskHandle);

    ESP_LOGI(TAG, "pmsx sensor initialized successfully");

    return ESP_OK;
}

static void pmsx_data_read_task(pmsx003_config_t* config) {
    uint8_t data[32];
    while(1) {
        pms_uart_read(config, data);
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

esp_err_t pms_uart_read(pmsx003_config_t* config, uint8_t *data) {
	int len = uart_read_bytes(config->uart_port, data, 32, 100 / portTICK_RATE_MS);
	if (config->enabled) {
		if (len >= 24 && data[0]==0x42 && data[1]==0x4d) {
				pm_data_t pm = decodepm_data(data, config->indoor ? 4 : 10);	//atmospheric from 10th byte, standard from 4th
				pm.sensor_id = config->sensor_id;
				if (config->callback) {
					config->callback(&pm);
				}
		} else if (len > 0) {
			ESP_LOGW(TAG, "invalid frame of %d", len);
			return ESP_FAIL;
		}
	}
	return ESP_OK;
}

static pm_data_t decodepm_data(uint8_t* data, uint8_t startByte) {
	pm_data_t pm = {
		.pm1_0 = ((data[startByte]<<8) + data[startByte+1]),
		.pm2_5 = ((data[startByte+2]<<8) + data[startByte+3]),
		.pm10 = ((data[startByte+4]<<8) + data[startByte+5])
	};
	return pm;
}

void idf_pmsx5003_destroy(pmsx003_config_t* config) {

    ESP_LOGI(TAG, "stop pmsx sensor");

    if (xReadTaskHandle != NULL) {
        vTaskDelete(xReadTaskHandle);
    }
    
    xReadTaskHandle = NULL;
    uart_driver_delete(config->uart_port);
}