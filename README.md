# idf-pms5003

Esp idf driver for pmsX003 sensor.
This library can be used with multiple sensors at the same time. For each sensor you can choose between
one shot or periodic data read.

# Configuration

Use idf.py menuconfig to configure RX_PIN, TX_PIN and SET_PIN of the sensor conected to your esp board.

# How to use

```c
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
    idf_pmsx5003_init(&pms_conf);
}
```

About the pmsx003_config_t config structure:

- sensor_id: returned in the callback to identify the sensor that generated the data.
- indoor: the sensor supports both indoor and outdoor data read.
- enabled: when set to true the sensor data will be read.
- periodic: true for periodic read. If set to false the sensor data will be read one time only.
- periodic_sec_interval: defines how often the sensor will be read in seconds. Min value is 35 (the sensor needs 30 seconds to return reliable results).


