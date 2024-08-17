/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "wifi/wifiDrv.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

#define MAXIMUM_RETRY  3



#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER ""


#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
EventBits_t bits;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        //con la anterior terminamos fase 3
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(TAG, "Desconectado de la red wifi");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } 
}

/*
funcion que se encarga de inicializar la conexion wifi
@param wlan: nombre de la red wifi
@param pwd: contraseña de la red wifi
@return
    - ESP_OK en caso de exito
    - ESP_FAIL en caso de fallo

*/  
esp_err_t wifi_init_sta(char *wlan, char *pwd)
{
    //fase 1
    //inicializamos el grupo de eventos
    //definiimos bits de eventos
    s_wifi_event_group = xEventGroupCreate();

    if(esp_netif_init() != ESP_OK){
        ESP_LOGE(TAG, "esp_netif_init failed");
        return ESP_FAIL;
    }
    
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //inicializacion por defecto de la interfaz wifi
    esp_netif_create_default_wifi_sta();

    //configuracion por defecto de la wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&cfg) != ESP_OK){
        ESP_LOGE(TAG, "esp_wifi_init failed");
        return ESP_FAIL;
    }
    

    //registramos los eventos
    //pendiente pro comprender mejor
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    if(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &event_handler,
                    NULL,
                    &instance_any_id) != ESP_OK){
        ESP_LOGE(TAG, "esp_event_handler_instance_register failed");
        return ESP_FAIL;
    }
    if(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &event_handler,
                    NULL,
                    &instance_got_ip) != ESP_OK){   
        ESP_LOGE(TAG, "esp_event_handler_instance_register failed");
        return ESP_FAIL;
    }


    //fase 2
    //configuracion de la wifi
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    //cargamos configuracion de la ssid y la contraseña suminitrada
    //en argumentos
    (void *)strcpy((char *)wifi_config.sta.ssid, (char *)wlan);
    (void *)strcpy((char *)wifi_config.sta.password, (char *)pwd);
    if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK){
        ESP_LOGE(TAG, "esp_wifi_set_mode failed");
        return ESP_FAIL;
    }
    if(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK){
        ESP_LOGE(TAG, "esp_wifi_set_config failed");
        return ESP_FAIL;
    }

    //puedo segmentar aca
    //
    //fase3 de la conexion

    if(esp_wifi_start() != ESP_OK){
        ESP_LOGE(TAG, "esp_wifi_start failed");
        return ESP_FAIL;
    }

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

return ESP_OK;
}

/*
funcion que se encarga de conectar el dispositivo a una red wifi
*/
esp_err_t wifi_connect(char *wlan, char *pwd)
{   
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    
    ret = wifi_init_sta(wlan, pwd);
    return ret;
}


/**
  * @brief  Disconnect from the currently connected Wi-Fi network.
  * @return
  *   - ESP_OK on success
  *   - ESP_FAIL en caso de fallo
  */
esp_err_t wifi_shutdown(void)
{
    if( esp_wifi_disconnect() != ESP_OK){
        return ESP_FAIL;
    }
    
    if(esp_wifi_stop() != ESP_OK){
        return ESP_FAIL;
    }

    if(esp_wifi_deinit() != ESP_OK){
        return ESP_FAIL;
    }
    return ESP_OK;
} 

esp_err_t wifi_status(void){
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Connected to %s", ap_info.ssid);
    } else {
        ESP_LOGI(TAG, "Not connected to any AP");
    }
    return ESP_OK;
} 

esp_err_t wifi_reconnect(void){
    esp_err_t err;
    err = esp_wifi_connect();
    return err;
}

/*
para una desconexion se debe llamar 
esp_wifi_connect(), para reconectar, el problema es uqe esto se
hace en el handler
mira
*/