/* PMSX0003 Sample

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

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
    .reset_pin = CONFIG_RESET_GPIO,
	.uart_tx_pin = CONFIG_TX_GPIO,
	.uart_rx_pin = CONFIG_RX_GPIO,
};

void app_main(void) {
    
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    idf_pmsx5003_init(&pms_conf);
}
