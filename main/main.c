#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt/mqtt.h"

#include "uSD/usdApp.h"
#include "i2c/i2c_app.h"
#include "bldc/bldc_servo.h"

//todo: verificar si el timestamp esta en segundos o milisegundos.

 
void app_main(void)
{
    xTaskCreate(i2c_app, "i2c_app", 4096, NULL, 10, NULL);
    //creamos cola para envio a la sd
    sd_App_queue = xQueueCreate(10, sizeof(struct Datos_usd));
    i2c_App_queue = xQueueCreate(10, sizeof(struct Datos_I2c));
    xTaskCreate(bldc_servo_app, "bldc_servo_app", 4096, NULL, 10, NULL);
    xTaskCreate(sd_App, "sd_App", 4096, (void *)&init_time, 10, NULL);
    



    for(;;)
    {   
        while(uxQueueMessagesWaiting(i2c_App_queue) > 0){
            struct Datos_usd datos_usd;
            struct Datos_I2c datos_i2c;
            xQueueReceive(i2c_App_queue, &datos_i2c, portMAX_DELAY);
            datos_usd.timestamp = esp_log_timestamp() + init_time;
            datos_usd.bldc = 15;
            datos_usd.presion = datos_i2c.presion;
            xQueueSend(sd_App_queue, &datos_usd, portMAX_DELAY);
        }
        vTaskDelay(1);
    }
}
//esp_log_timestamp(void)

/*
Para pruebas de mqtt 

printf("Run!\n");
mqtt_initVars("NATALIA", "1128418683", "mqtt://3.90.24.183", 8924);


for(;;)
    {   
     MSmqtt();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


*/