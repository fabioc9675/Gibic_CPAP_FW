#include "uSD/usdApp.h"


char buffer[100];

void sdApp(uint32_t contador){
     FILE* f = fopen("/sdcard/hello2.txt", "a");
    if (f == NULL) {
        printf("Failed to open file for writing\n");
        return;
    }

    sprintf(buffer, "Contador_2: %ld\n", contador);
    // Escribe datos en el archivo
    fprintf(f, buffer);

    // Cierra el archivo
    fclose(f);

    printf("File written\n");

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

