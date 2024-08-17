#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "uSD/usdDrv.h"
#include "uSD/usdApp.h"

uint32_t contador = 0;
//char buffer[100];


void app_main(void)
{
    //init_sdmmc();
    printf("Run!\n");


    for(;;)
    {   
    
        /*
            // Abre un archivo para escribir modo append
        FILE* f = fopen("/sdcard/hello.txt", "a");
        if (f == NULL) {
            printf("Failed to open file for writing\n");
            return;
        }

        sprintf(buffer, "Contador: %ld\n", contador);
        // Escribe datos en el archivo
        fprintf(f, buffer);

        // Cierra el archivo
        fclose(f);

        printf("File written\n");


void app_main(void)
{
    gpio_evt_queue = xQueueCreate(25, sizeof(uint32_t));
    xTaskCreate(app2_gpio, "app_gpio", 4096, NULL, 10, &gpioHandle);
    //            ssid    pwd_ssid          broker                 puerto    user     pwd
    mqtt_initVars("ELECTRONICA SID", "electro424", "mqtt://3.90.24.183", 8924);
    
    for(;;){
  
        MSmqtt();
        vTaskDelay(1);

    }
}



        */

        //sdApp(contador);
        //contador++;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
