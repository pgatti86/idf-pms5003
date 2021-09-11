#include "idf-pmsx003.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PMS_FRAME_LEN 32

static const char* TAG = "pmsx003";

static const int pmsx_baud_rate = 9600;

static const int uart_buffer_size = 1024 * 2;

static TaskHandle_t xReadTaskHandles[UART_NUM_MAX];

/*---------------- STATIC FUNCTIONS DEFs ----------------------------------*/
static void pmsx_data_read_task();

static esp_err_t pms_uart_read(pmsx003_config_t *config, uint8_t *data);

static int calculate_frame_checksum(uint8_t *data);

static bool is_pmsx_frame_valid(uint8_t *data, int data_len);

static pm_data_t decode_pm_data(uint8_t *data, bool indoor);
/*--------------------------------------------------------------------------*/

esp_err_t idf_pmsx5003_init(pmsx003_config_t *config) {

    if (xReadTaskHandles[config->uart_port] != NULL) {
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

    xTaskCreate(pmsx_data_read_task, "data read task", 2048, config, 5, &(xReadTaskHandles[config->uart_port]));

    ESP_LOGI(TAG, "pmsx sensor initialized successfully");

    return ESP_OK;
}

static void pmsx_data_read_task(pmsx003_config_t* config) {
    uint8_t data[PMS_FRAME_LEN];
    while (1) {
        if (config->enabled) {
            pms_uart_read(config, data);
        }
        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

esp_err_t pms_uart_read(pmsx003_config_t *config, uint8_t *data) {
	
    size_t available_data;
    uart_get_buffered_data_len(config->uart_port, &available_data);
    if (available_data < PMS_FRAME_LEN) {
        ESP_LOGW(TAG, "not enought data available");
        return ESP_FAIL;
    }

    int data_len = uart_read_bytes(config->uart_port, data, PMS_FRAME_LEN, 100 / portTICK_RATE_MS);    
    if (is_pmsx_frame_valid(data, data_len)) {
        pm_data_t pm = decode_pm_data(data, config->indoor);
        pm.sensor_id = config->sensor_id;
        if (config->callback) {
            config->callback(&pm);
        }
    } else {
        ESP_LOGW(TAG, "checksum mismatch");
        return ESP_FAIL;
    }

	return ESP_OK;
}

static bool is_pmsx_frame_valid(uint8_t *data, int data_len) {

    if (data_len==PMS_FRAME_LEN && data[0]==0x42 && data[1]==0x4d) {
        
        int calculated_checksum = calculate_frame_checksum(data);
        int frame_checksum = ((data[30]<<8) + data[31]);
       
        return calculated_checksum == frame_checksum;
    }

    return false;
}

static int calculate_frame_checksum(uint8_t *data) {
    int sum = 0;
    for (uint8_t i=0; i<(PMS_FRAME_LEN -2); i++) {
        sum += data[i];
    }
    return sum;
}

static pm_data_t decode_pm_data(uint8_t* data, bool indoor) {
	
    uint8_t start_byte = indoor ? 4 : 10;
    
    pm_data_t pm = {
		.pm1_0 = ((data[start_byte]<<8) + data[start_byte+1]),
		.pm2_5 = ((data[start_byte+2]<<8) + data[start_byte+3]),
		.pm10 = ((data[start_byte+4]<<8) + data[start_byte+5]),
        .particles_03um = ((data[16]<<8) + data[17]),
        .particles_05um = ((data[18]<<8) + data[19]), 
        .particles_10um = ((data[20]<<8) + data[21]), 
        .particles_25um = ((data[22]<<8) + data[23]), 
        .particles_50um = ((data[24]<<8) + data[25]), 
        .particles_100um = ((data[26]<<8) + data[27])
	};
	return pm;
}

void idf_pmsx5003_destroy(pmsx003_config_t* config) {

    ESP_LOGI(TAG, "stop pmsx sensor");

    if (xReadTaskHandles[config->uart_port] != NULL) {
        vTaskDelete(xReadTaskHandles[config->uart_port]);
    }
    
    xReadTaskHandles[config->uart_port] = NULL;
    uart_driver_delete(config->uart_port);
}