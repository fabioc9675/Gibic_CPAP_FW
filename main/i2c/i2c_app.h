#ifndef I2C_APP_H
#define I2C_APP_H

#include "i2c/i2c_drv/i2c_common.h"
#include <time.h>

extern QueueHandle_t i2c_App_queue;
extern time_t init_time;

struct Datos_I2c{
    float presion;
};


void i2c_app(void *pvParameters);
void i2c_app_read(void);

typedef enum {
    st_init,
    st_reqAdc0,
    
    st_rAdc0
} i2c_stetes_t;









#endif // I2C_APP_H