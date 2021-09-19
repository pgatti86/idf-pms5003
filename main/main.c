/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "idf-pmsx003.h"

static const char* TAG = "MAIN";

static void pms_callback(pm_data_t *sensor_data) {
    ESP_LOGI(TAG, "pm10: %d ug/m3", sensor_data->pm10);
    ESP_LOGI(TAG, "pm2.5: %d ug/m3", sensor_data->pm2_5);
    ESP_LOGI(TAG, "pm1.0: %d ug/m3", sensor_data->pm1_0);
    ESP_LOGI(TAG, "particles > 0.3um / 0.1L: %d", sensor_data->particles_03um);
    ESP_LOGI(TAG, "particles > 0.5um / 0.1L: %d", sensor_data->particles_05um);
    ESP_LOGI(TAG, "particles > 1.0um / 0.1L: %d", sensor_data->particles_10um);
    ESP_LOGI(TAG, "particles > 2.5um / 0.1L: %d", sensor_data->particles_25um);
    ESP_LOGI(TAG, "particles > 5.0um / 0.1L: %d", sensor_data->particles_50um);
    ESP_LOGI(TAG, "particles > 10.0um / 0.1L: %d", sensor_data->particles_100um);
}

pmsx003_config_t pms_conf = {
    .sensor_id = 1,
    .uart_port = UART_NUM_2,
	.indoor = false,
    .enabled = true,
    .periodic = true,
    .periodic_sec_interval = 300, // every five minutes
	.callback = &pms_callback,
	.set_pin = CONFIG_SET_GPIO,
	.uart_tx_pin = CONFIG_TX_GPIO,
	.uart_rx_pin = CONFIG_RX_GPIO,
};

void app_main(void) {
    
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    idf_pmsx5003_init(&pms_conf);
}
