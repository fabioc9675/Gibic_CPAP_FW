#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "uSD/usdDrv.h"
#include "uSD/usdApp.h"
#include "mqtt/mqtt.h"



void app_main(void)
{
    //init_sdmmc();
    printf("Run!\n");


    //mqtt_initVars("NATALIA", "1128418683", "mqtt://3.90.24.183", 8924);
    //mqtt_initVars("DAH", "3006658465", "mqtt://3.90.24.183", 8924);
    mqtt_initVars("ELECTRONICA SID", "electro424", "mqtt://3.90.24.183", 8924);
    for(;;)
    {   

        /* para pruebas de sd descomentar las siguientes lineas
        */
        //sdApp(contador);
        //contador++;
    

/*

    mqtt_initVars("ELECTRONICA SID", "electro424", "mqtt://3.90.24.183", 8924);
    
    for(;;){
  
        
        vTaskDelay(1);


        */

        MSmqtt();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
