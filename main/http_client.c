#include "http_client.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#define TAG "HTTP"

#define IPSTACK_KEY CONFIG_ESP_IPSTACK_KEY
#define OPENWEATHER_KEY CONFIG_ESP_OPENWEATHER_KEY

double latitude, longitude;
char memoJSON[800] = {0};

void parse_ipstackJSON() {
    cJSON *ipstack_json = cJSON_Parse(memoJSON);
    const cJSON *lat = NULL;
    const cJSON *lon = NULL;

    lat = cJSON_GetObjectItemCaseSensitive(ipstack_json, "latitude");
    lon = cJSON_GetObjectItemCaseSensitive(ipstack_json, "longitude");

    if(cJSON_IsNumber(lat)) {
        latitude = lat->valuedouble;
    }

    if(cJSON_IsNumber(lon)) {
        longitude = lon->valuedouble;
    }

    cJSON_Delete(ipstack_json);
}

void parse_openweatherJSON() {
    cJSON *openweather_json = cJSON_Parse(memoJSON);

    const cJSON *label_main = NULL;
    const cJSON *temp_atual = NULL;
    const cJSON *temp_maxima = NULL;
    const cJSON *temp_minima = NULL;
    const cJSON *humidity = NULL;

    label_main = cJSON_GetObjectItemCaseSensitive(openweather_json, "main");

    temp_atual = cJSON_GetObjectItemCaseSensitive(label_main, "temp");
    temp_maxima = cJSON_GetObjectItemCaseSensitive(label_main, "temp_max");
    temp_minima = cJSON_GetObjectItemCaseSensitive(label_main, "temp_min");
    humidity = cJSON_GetObjectItemCaseSensitive(label_main, "humidity");

    if(cJSON_IsNumber(temp_atual)) {
        printf("Temperatura Atual: %.2lf\n", temp_atual->valuedouble);
    }

    if(cJSON_IsNumber(temp_maxima)) {
        printf("Temperatura Maxima: %.2lf\n", temp_maxima->valuedouble);
    }

    if(cJSON_IsNumber(temp_minima)) {
        printf("Temperatura Minima: %.2lf\n", temp_minima->valuedouble);
    }

    if(cJSON_IsNumber(humidity)) {
        printf("Umidade: %.2lf%%\n", humidity->valuedouble);
    }

    cJSON_Delete(openweather_json);
}

esp_err_t _http_event_handle_ipstack(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            //ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            //ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            //printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            strncat(memoJSON, (char*)evt->data, evt->data_len);
            //printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_FINISH:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            parse_ipstackJSON();
            memoJSON[0] = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            //ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

esp_err_t _http_event_handle_openweather(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            //ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            //ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            //printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            strncat(memoJSON, (char*)evt->data, evt->data_len);
            //printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_FINISH:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            parse_openweatherJSON();
            memoJSON[0] = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            //ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void https_request_ipstack() {
    const char url[] = "http://api.ipstack.com/check?access_key=";

    char *result = malloc(strlen(url) + strlen(IPSTACK_KEY) + 1);
    strcpy(result, url);
    strcat(result, IPSTACK_KEY);

    esp_http_client_config_t config = {
        .url = result,
        .event_handler = _http_event_handle_ipstack,
        //.cert_pem = howsmyssl_com_root_cert_pem_start,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    /*
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    */

    free(result);
    esp_http_client_cleanup(client);
}

void https_request_openweather() {
    char url[130];

    sprintf(url, "http://api.openweathermap.org/data/2.5/weather?lat=%lf&lon=%lf&appid=%s", latitude, longitude, OPENWEATHER_KEY);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handle_openweather,
        //.cert_pem = howsmyssl_com_root_cert_pem_start,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    /*
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    */

    esp_http_client_cleanup(client);
}
