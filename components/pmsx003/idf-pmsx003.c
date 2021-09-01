#include "idf-pmsx003.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "pmsx003";

static const int pmsx_baud_rate = 9600;

static const uart_port_t uart_number = UART_NUM_2;

static TaskHandle_t xReadTaskHandle = NULL;

static void pmsx_data_read_task() {
    
    while(1) {
    }
    
    vTaskDelete(NULL);
}

esp_err_t idf_pmsx5003_init(void) {

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

    esp_err_t result = uart_param_config(uart_number, &uart_config);
    if (result != ESP_OK)
        return ESP_FAIL;
    
    result = uart_set_pin(uart_number, CONFIG_TX_GPIO, CONFIG_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (result != ESP_OK)
        return ESP_FAIL;

    const int uart_buffer_size = (1024 * 2);    
    result = uart_driver_install(uart_number, uart_buffer_size, uart_buffer_size, 0, NULL, 0);
    if (result != ESP_OK)
        return ESP_FAIL;

    xTaskCreate(pmsx_data_read_task, "data read task", 2048, NULL, 5, &xReadTaskHandle);

    ESP_LOGI(TAG, "pmsx sensor initialized successfully");

    return ESP_OK;
}

void idf_pmsx5003_destroy(void) {

    ESP_LOGI(TAG, "stop pmsx sensor");

    if (xReadTaskHandle != NULL) {
        vTaskDelete(xReadTaskHandle);
    }
    
    xReadTaskHandle = NULL;
    uart_driver_delete(uart_number);
}