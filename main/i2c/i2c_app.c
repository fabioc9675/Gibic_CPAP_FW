#include "i2c/i2c_app.h"

i2c_master_bus_handle_t I2C1_bus_handle;
QueueHandle_t i2c_App_queue = NULL;

// for i2c1
i2c_master_bus_config_t i2c1_bus_conf = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_DEV1,
    .scl_io_num = SCL1,
    .sda_io_num = SDA1,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = 1,
};

i2c_master_dev_handle_t ds_handle;
/*
 *@brief I2C1 master initialization
 */
esp_err_t I2C1_init(void)
{
    esp_err_t ret;
    ret = i2c_new_master_bus(&i2c1_bus_conf, &I2C1_bus_handle);
    //ESP_LOGI(TAG, "init i2c %s", ret == ESP_OK ? "success" : "failed");
    return ret;
}


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
float offsetPresion = 0;
time_t init_time=0;

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

    //leemos offset presion
    (void)i2c_adc1015_get_ch(3, &adc);
    ret = i2c_adc1015_read_ch(&adc);
    while (ret != ESP_OK) {
        ret = i2c_adc1015_read_ch(&adc);    
    }
    offsetPresion = (float)(((adc)/(0.2*3000))-1);
    
    for(;;) 
    {
        (void)i2c_adc1015_get_ch(3, &adc);
        ret = i2c_adc1015_read_ch(&adc);
        while (ret != ESP_OK) {
            ret = i2c_adc1015_read_ch(&adc);    
            //ESP_LOGI("APP_READ", "In while");
        }
        datos.presion = ((adc)/(0.2*3000))-1;
        datos.presion -= offsetPresion;
        datos.presion *= 10;
        xQueueSend(i2c_App_queue, &datos, 0);
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
      device init error");
        return;
    }
   

*/
}

/*
    intercalar lectura del adc con la de los otros dispositivos 
    presentes, teneiendo en cuanta que el adc tiene un pin de 
    interrupcion que se activa cuando cuando la conversion esta lista
*/