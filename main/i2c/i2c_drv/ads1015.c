#include "i2c/i2c_drv/i2c_common.h"

uint8_t adc1015Buf[2]={0};

i2c_adc1015_config_t adc1015_conf = {
    .conf.scl_speed_hz = I2C_FREQ_HZ,
    .conf.device_address = ADC_ADDR,
    .conf.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .dl_p_addr = 1,
    .dl_r = 2,
    .buff=adc1015Buf,
    .wt_ms = 1,
};


esp_err_t i2c_adc1015_init(){
    uint8_t adctemp[3];
    esp_err_t ret;
    ret = i2c_master_bus_add_device(I2C1_bus_handle, &adc1015_conf.conf, &adc1015_conf.handle);
    //ESP_LOGI(TAG, "init adc1015_conf %s", ret == ESP_OK ? "success" : "failed");
    /*  
     * b11-b9 = 001   ±4.096V range
     * b8     = 1     config single shot mode
     * b7-b5  = 111   3300sps
     * 
     */ 
    if (ret == ESP_OK){
        adctemp[0] = 0x01; //puntero a configuracion
        adctemp[1] = 0x03;
        adctemp[2] = 0xE0;
     
        //escritura de configuracion
        ret = i2c_master_transmit( adc1015_conf.handle, 
                                   adctemp,3, -1);
        
        //lectura de configuracion
        adc1015_conf.p_addr = 0x01;
        adc1015Buf[0] = 0;
        adc1015Buf[1] = 0;
       
        ret = i2c_master_transmit_receive( adc1015_conf.handle, 
                                       &adc1015_conf.p_addr,adc1015_conf.dl_p_addr,
                                       adc1015_conf.buff, adc1015_conf.dl_r, -1);
        //ESP_LOGI(TAG, "ADC1015 read data init: %02x,%02X", 
        //    adc1015Buf[0],adc1015Buf[1]);
    }

    return ret;
}

esp_err_t i2c_adc1015_get_ch(uint8_t ch, uint16_t *data){
    uint8_t adctemp[3];
    esp_err_t ret;

    //read config register
    adc1015_conf.p_addr = 0x01;
    ret = i2c_master_transmit_receive( adc1015_conf.handle, 
                                       &adc1015_conf.p_addr,adc1015_conf.dl_p_addr,
                                       adc1015_conf.buff, adc1015_conf.dl_r, -1);

    //write config register
    adc1015Buf[0] &= 0x0F;
    adc1015Buf[0] |= ((--ch+4) << 4);
    adc1015Buf[0] |= 0x80;

    //revisar para reutilizar el vector del arregolo,
    // se debe jugar con el tamaño, ampliarlo a 3 
    adctemp[0] = 0x01;
    adctemp[1] = adc1015Buf[0];
    adctemp[2] = adc1015Buf[1];

    ret = i2c_master_transmit( adc1015_conf.handle, 
                                   adctemp,3, -1);
    
    adc1015Buf[0] = 0;
    adc1015Buf[1] = 0;
    ret = i2c_master_transmit_receive( adc1015_conf.handle, 
                                    &adc1015_conf.p_addr,adc1015_conf.dl_p_addr,
                                    adc1015_conf.buff, adc1015_conf.dl_r, -1);
    //ESP_LOGI(TAG, "ADC1015 status data: %02x,%02X", adc1015Buf[0],adc1015Buf[1]);
    *data = (adc1015Buf[0] << 8) | adc1015Buf[1];
    return ret;

}

esp_err_t i2c_adc1015_read_ch(uint16_t *data){
    esp_err_t ret;

    //read config register
    adc1015_conf.p_addr = 0x01;
    ret = i2c_master_transmit_receive( adc1015_conf.handle, 
                                       &adc1015_conf.p_addr,adc1015_conf.dl_p_addr,
                                       adc1015_conf.buff, adc1015_conf.dl_r, -1);

    //ESP_LOGI(TAG, "ADC1015 read data: %02x,%02X", adc1015Buf[0],adc1015Buf[1]);
    if(adc1015Buf[0] & 0x80){
        //lectura completa
        //se procede a la lectura
        adc1015_conf.p_addr = 0x00;
        adc1015Buf[0] = 0;
        adc1015Buf[1] = 0;
        ret = i2c_master_transmit_receive( adc1015_conf.handle, 
                                       &adc1015_conf.p_addr,adc1015_conf.dl_p_addr,
                                       adc1015_conf.buff, adc1015_conf.dl_r, -1);
        //ESP_LOGI(TAG, "ADC1015 read data: %02x,%02X", adc1015Buf[0],adc1015Buf[1]);                               
        *data = ((adc1015Buf[0] << 8) | adc1015Buf[1]) >> 4;
        *data *= 2; //mv;
        

    }else{
        *data = 0;
        ret = ESP_ERR_NOT_FINISHED;
    }
    return ret;
}