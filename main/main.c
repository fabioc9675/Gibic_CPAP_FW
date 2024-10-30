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
#include "uart/uartapp.h"

//todo: verificar si el timestamp esta en segundos o milisegundos.

uint16_t bldc_sp = 100;
void app_main(void)
{
    
    struct Datos_usd datos_usd;

    //creamos cola para envio a la sd
    sd_App_queue = xQueueCreate(10, sizeof(struct Datos_usd));
    i2c_App_queue = xQueueCreate(10, sizeof(struct Datos_I2c));
    bldc_App_queue = xQueueCreate(10, sizeof(uint16_t));
    uart_app_queue = xQueueCreate(10, sizeof(uint8_t));
   
    //cramos tareas
    xTaskCreate(i2c_app, "i2c_app", 4096, NULL, 10, NULL);
    xTaskCreate(bldc_servo_app, "bldc_servo_app", 4096, NULL, 10, NULL);
    xTaskCreate(sd_App, "sd_App", 4096, (void *)&init_time, 10, NULL);
    xTaskCreate(uart_app, "uart_app", 4096, NULL, 10, NULL);
    


    xQueueSend(bldc_App_queue, &bldc_sp, 0);
    init_time *= 1000;
    for(;;)
    {   
        while(uxQueueMessagesWaiting(i2c_App_queue) > 0){
            
            struct Datos_I2c datos_i2c;
            xQueueReceive(i2c_App_queue, &datos_i2c, 0);
            datos_usd.timestamp = esp_log_timestamp() + init_time;
            //datos_usd.bldc = 15;
            
            datos_usd.presion = datos_i2c.presion;
            xQueueSend(sd_App_queue, &datos_usd, 0);
        }

        while(uxQueueMessagesWaiting(uart_app_queue) > 0){
            uint8_t inByte;
            xQueueReceive(uart_app_queue, &inByte, 0);
            //escalamos la presion
            datos_usd.bldc = inByte;
            bldc_sp = (inByte-3)* 55;
            xQueueSend(bldc_App_queue, &bldc_sp, 0);
            
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