#include "uSD/usdApp.h"

//handler de la cola de envio a la sd
QueueHandle_t sd_App_queue = NULL;

char buffer[100];
char file_log[17];

void sd_App(void *pvParameters){
    time_t *time = (uint32_t *)pvParameters;
    
    init_sdmmc();

    struct tm *tm_info = localtime(&time);
    strftime(&file_log[4], 9, "%Y%m%d", tm_info);
    file_log[0] = '/';
    file_log[1] = 's';
    file_log[2] = 'd';
    file_log[3] = '/';
    strcat(file_log, ".csv");

    // Abre un archivo para escribir modo append
    FILE* f = fopen(file_log, "a");

    //FILE* f = fopen("/sd/hello2.txt", "a");
    if (f == NULL) {
        //enviamos mensaje de error
        ESP_LOGE("SD_APP", "Failed to open file for writing\n");
        ESP_LOGE("SD_APP", "File: %s\n", file_log); 
        return;
    }else{
        sprintf(buffer, "Timestamp; BLDC %%; Presion mmH2O\n");
        fprintf(f, buffer);
        fclose(f);
        ESP_LOGI("SD_APP", "File written\n");    
    }

    /*sprintf(buffer, "Contador_2: %d\n", 1);
    // Escribe datos en el archivo
    fprintf(f, buffer);

    // Cierra el archivo
    fclose(f);

    printf("File written\n");*/
    for(;;)
    {
        struct Datos_usd datos;
        while(uxQueueMessagesWaiting(sd_App_queue) > 0){
            xQueueReceive(sd_App_queue, &datos, portMAX_DELAY);
            FILE* f = fopen(file_log, "a");
            if (f == NULL) {
                //enviamos mensaje de error
                ESP_LOGE("SD_APP", "Failed to open file for writing\n");
                ESP_LOGE("SD_APP", "File: %s\n", file_log); 
                return;
            }else{}
                sprintf(buffer, "%ld; %d; %f\n", datos.timestamp, datos.bldc, datos.presion);
                // Escribe datos en el archivo
                fprintf(f, buffer);
                // Cierra el archivo
                fclose(f);
                //printf(buffer);
        }
        //printf(file_log);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

}

/*void app_main(void)
{
    init_sdmmc();
    printf("tarjeta montada!\n");

    for(;;)
    {   
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

    contador++;

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}*/

