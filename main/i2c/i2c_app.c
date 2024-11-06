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

i2c_stetes_t i2c_state = st_init;

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
    float sdppresiondiff = 0;
    float sdptemperatura = 0;

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

    if(ESP_OK !=xSdp810Init()){
        printf("error en sdp810\n");
    };
    if(ESP_OK !=xSdp810_StartContinousMeasurement(SDP800_TEMPCOMP_MASS_FLOW, SDP800_AVERAGING_TILL_READ)){
        printf("error 2 en sdp810\n");
    };
    
    for(;;) 
    {

        switch (i2c_state)
        {

            case st_init:
                i2c_state = st_reqAdc0;
                break;


                
            case st_reqAdc0: //request presion
                (void)i2c_adc1015_get_ch(3, &adc);
                i2c_state = st_rsdp810;
                /* code */
                break;

            case st_rsdp810: //read sdp810
                //(void)xSdp810_ReadMeasurementResults(&sdppresiondiff, &sdptemperatura);
                datos.flujo = sdppresiondiff;
                printf("presion: %f\n", sdppresiondiff);
                i2c_state = st_rAdc0;
                break;

            case st_rAdc0: //read presion
                ret = i2c_adc1015_read_ch(&adc);
                while (ret != ESP_OK) {
                    ret = i2c_adc1015_read_ch(&adc); 
                    printf("esperando conversion adc\n");   
                }
                i2c_state = st_iddle;
                break;   

            case st_iddle:
                datos.presion = ((adc)/(0.2*3000))-1;
                datos.presion -= offsetPresion;
                datos.presion *= 10;
                datos.flujo = sdppresiondiff;
                xQueueSend(i2c_App_queue, &datos, 0);
                vTaskDelay(50 / portTICK_PERIOD_MS);
                i2c_state = st_reqAdc0;
                break;

            default:
            break;
        }



      

    }
}

/*
    intercalar lectura del adc con la de los otros dispositivos 
    presentes, teneiendo en cuanta que el adc tiene un pin de 
    interrupcion que se activa cuando cuando la conversion esta lista
*/