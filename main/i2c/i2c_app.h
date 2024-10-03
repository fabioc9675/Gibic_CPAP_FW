#ifndef I2C_APP_H
#define I2C_APP_H

#include "i2cdrv.h"
#include <time.h>

extern QueueHandle_t i2c_App_queue;
extern time_t init_time;

struct Datos_I2c{
    float presion;
};


void i2c_app(void *pvParameters);
void i2c_app_read(void);









#endif // I2C_APP_H