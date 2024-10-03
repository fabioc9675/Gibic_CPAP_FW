#include "i2c/i2c_app.h"

QueueHandle_t i2c_App_queue = NULL;
/*
 *todo: segmentar la aplicacion en drivers para
    * cada dispositivo i2c
 
 *todo: maquina de estados
    * 1. Init I2C1
    * 2. Init DS1338
    * 3. Init ADC1015
    * 4. Read DS1338
    * 5. Read ADC1015
    * 6. Print data
    * 7. Loop 
    * 8. delay to 50ms
    * 9. goto 4
 */


uint16_t adc=0;
time_t init_time=0;


//void i2c_app_read(void)
//{
   // (void)i2c_ds1338_read();
   // (void)i2c_adc1015_get_ch(1, &adc);
   // ESP_LOGI("APP_READ", "ADC1015 read data: %d", adc);
//}


void i2c_app(void *pvParameters)
{
    esp_err_t ret;
    struct Datos_I2c datos;

    (void)I2C1_init();

    //init and read rtc
    (void)i2c_ds1338_init();
    (void)i2c_ds1338_read(&init_time);
    
    //init adc
    (void)i2c_adc1015_init();
    
    while (1) {
        
        (void)i2c_adc1015_get_ch(3, &adc);
        ret = i2c_adc1015_read_ch(&adc);
        while (ret != ESP_OK) {
            ret = i2c_adc1015_read_ch(&adc);    
            //ESP_LOGI("APP_READ", "In while");
        }
        //ESP_LOGI("APP_READ", "ADC1015 read data: %dmV", adc);
        datos.presion = (((float)adc/1000)-(float)0.6)/(float)0.006;
        xQueueSend(i2c_App_queue, &datos, portMAX_DELAY);
        //ESP_LOGI("APP_READ", "presion: %f mmH2O", presion);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    /*
    i2c_master_bus_handle_t i2c1_bus_h;
    i2c_master_bus_handle_t i2c2_bus_h;
    i2c_ds1338_handle_t ds1338_dev;
    i2c_ds1338_config_t ds1338_config = {
        .ds1338_dev = {
            .dev_addr = RTC_ADDR,
            .addr_wordlen = 1,
            .timeout = I2C_TIMEOUT_MS,
            .freq = I2C_FREQ_HZ,
        },
        .data_len = 8,
        .write_time_ms = 10,
    };
    esp_err_t ret;
    ret = i2c_new_master_bus(&i2c1_bus_conf, &i2c1_bus_h);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C1 master bus create error");
        return;
    }
    ret = i2c_new_master_bus(&i2c2_bus_conf, &i2c2_bus_h);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C2 master bus create error");
        return;
    }
    ret = i2c_ds1338_init(i2c1_bus_h, &ds1338_config, &ds1338_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "DS1338 device init error");
        return;
    }
    ret = i2c_ds1338_read(ds1338_dev, 0, ds1338Buf, 8);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "DS1338 read error");
        return;
    }
    for (int i = 0; i < 8; i++) {
        ESP_LOGI(TAG, "DS1338 read data[%d]: %02x", i, ds1338Buf[i]);
    }
    ret = i2c_ds1338_write(ds1338_dev, 0, ds1338Buf, 8);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "DS1338 write error");
        return;
    }
    ret = i2c_ds1338_read(ds1338_dev, 0, ds1338Buf, 8);
    if (ret != ESP_OK) {
        ESP_LOGE

*/
}

/*
    intercalar lectura del adc con la de los otros dispositivos 
    presentes, teneiendo en cuanta que el adc tiene un pin de 
    interrupcion que se activa cuando cuando la conversion esta lista
*/