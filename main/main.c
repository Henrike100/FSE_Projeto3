#include <stdio.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED 2

xSemaphoreHandle conexaoWifiSemaphore;
int conectado;

void taskLED(void * params) {
    int estado = 0;

    while(true) {
        if(conectado) {
            gpio_set_level(LED, 1);
        }
        else {
            gpio_set_level(LED, estado);
            estado = !estado;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void RealizaHTTPRequest(void * params) {
    while(true) {
        if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY)) {
            //ESP_LOGI("Main Task", "Realiza HTTP Request");
            https_request();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();

    xTaskCreate(&taskLED, "Cuida do LED", 1024, NULL, 1, NULL);

    wifi_start();

    xTaskCreate(&RealizaHTTPRequest, "Processa HTTP", 4096, NULL, 1, NULL);
}
