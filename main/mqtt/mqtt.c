/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "mqtt.h"

static const char *TAG = "mqtt_example";
esp_mqtt_client_handle_t client;

//var maquina de estados
msMqtt mqtt_state = stConnectWifi;

//var para wifi y mqtt
char *ssid;
char *pwdSsid;
char *brokerAddress; 
uint16_t portBroker;
uint8_t flsendmqtt = 0;


//contadores
uint32_t cntEv1 = 0;
uint32_t cntEv2 = 0;
char strcont[10];
char strcont2[30];
uint8_t mac[6];

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
   
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_state = stReconnectWifi;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_err_t mqtt_app_start(char *broker, uint32_t port)
{
    
    esp_err_t err;
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = " ",
        .broker.address.port = port,
        .session.keepalive = 30,
        //.credentials.username = "mqtt",
        //.credentials.authentication.password = "mqtt",
        .credentials.set_null_client_id = false,
        .credentials.username = "cpap_publisher",
        .credentials.client_id = "ABC123",
        .credentials.authentication.password = "Cp998*-Tx",
    };
    (void *)strcpy((char *)mqtt_cfg.broker.address.uri, (char *)broker);


    //esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    err = esp_mqtt_client_start(client);
    return err;
}

void mqtt_initVars(char *wlan, char *pwd, char *broker, uint32_t port)
{   
    ssid = wlan;
    pwdSsid = pwd;
    brokerAddress = broker;
    portBroker = port;
}

void send_mqtt_message(char *topic, char *message)
{
    (void)esp_mqtt_client_enqueue(client, topic, message, 0, 1, 0, 1);
}

//maquina de estados para gestionar conexion y envio de datos
// funcion para gestionar la cola de las int y manejar contadores
//definar tags mqtt_app_start

void MSmqtt(){
    esp_err_t err = ESP_FAIL;
    uint32_t io_num;

/*
    if(xQueueReceive(gpio_evt_queue, &io_num, 0)){
        flsendmqtt = 1;
        switch (io_num)
        {
            case 1:
                cntEv1++;
                break;
            case 2:
                cntEv2++;
                break;
            default:
                break;
        }
        ESP_LOGI(TAG, "recibiendo datos de cola");

    }*/

    switch(mqtt_state){

        case stConnectWifi:
            err = wifi_connect (ssid, pwdSsid);
            if(err == ESP_OK){
                mqtt_state = stConnectMqtt;
            }
            ESP_LOGI(TAG, "stConnectWifi");
            break;
        
        case stReconnectWifi:
            err = wifi_reconnect();
            if(err == ESP_OK){
                mqtt_state = stSend;
            }
            break;

        case stConnectMqtt:
            err = mqtt_app_start(brokerAddress, portBroker);
            if(err == ESP_OK){
                mqtt_state = stSend;
            }//else{
               // mqtt_state = stConnectWifi; 
            //}
            ESP_LOGI(TAG, "stConnectMqtt");
            break;

        case stSend:
            //send_mqtt_message("/topic/p1", "asd");
            if(flsendmqtt == 1){
                flsendmqtt = 0;
                esp_read_mac(mac,ESP_MAC_WIFI_STA);
                
                sprintf(strcont2, "%02X:%02X:%02X:%02X:%02X:%02X/E1", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                sprintf(strcont, "%ld",cntEv1);
                send_mqtt_message(strcont2, strcont);
                
                sprintf(strcont2, "%02X:%02X:%02X:%02X:%02X:%02X/E2", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                sprintf(strcont, "%ld",cntEv2);
                send_mqtt_message(strcont2, strcont);
            }
            break;


        default:
            break;  
    }
}