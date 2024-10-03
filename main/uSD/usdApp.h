#ifndef USD_APP_H
#define USD_APP_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "usdDrv.h"

void sdApp(uint32_t contador);

struct Datos_usd{
    uint32_t timestamp;
    uint8_t bldc;
    float presion;
};

void sd_App(void *pvParameters);
extern QueueHandle_t sd_App_queue;

#endif // USD_APP_H