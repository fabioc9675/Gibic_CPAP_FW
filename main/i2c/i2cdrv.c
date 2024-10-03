#include "i2c/i2cdrv.h"
#include <time.h>


static const char TAG[] = "ds1338";

uint8_t dds1338Buf[7]={0};
uint8_t adc1015Buf[2]={0};
// for i2c1
i2c_master_bus_config_t i2c1_bus_conf = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_DEV1,
    .scl_io_num = SCL1,
    .sda_io_num = SDA1,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = 1,
};
i2c_master_bus_handle_t I2C1_bus_handle;


i2c_ds1338_config_t ds1338_conf = {
    .conf.scl_speed_hz = I2C_FREQ_HZ,
    .conf.device_address = RTC_ADDR,
    .conf.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .addr = 0,
    .dl_addr = 1,
    .dl_r = 7,
    .buff=dds1338Buf,
    .wt_ms = 1,
};


i2c_adc1015_config_t adc1015_conf = {
    .conf.scl_speed_hz = I2C_FREQ_HZ,
    .conf.device_address = ADC_ADDR,
    .conf.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .dl_p_addr = 1,
    .dl_r = 2,
    .buff=adc1015Buf,
    .wt_ms = 1,
};

/*
i2c_device_config_t ds1338_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = RTC_ADDR,
    .scl_speed_hz = I2C_FREQ_HZ,
};*/


//i2c_ds1338_t ds1338_handle;  &ds1338_handle.i2c_dev

i2c_master_dev_handle_t ds_handle;
/*
 *@brief I2C1 master initialization
 */
esp_err_t I2C1_init(void)
{
    esp_err_t ret;
    ret = i2c_new_master_bus(&i2c1_bus_conf, &I2C1_bus_handle);
    ESP_LOGI(TAG, "init i2c %s", ret == ESP_OK ? "success" : "failed");
    return ret;
}

esp_err_t i2c_ds1338_init(){
    esp_err_t ret;
    //ret = i2c_master_bus_add_device(I2C1_bus_handle, &ds1338_conf.conf, &ds1338_conf.handle);
    ret = i2c_master_bus_add_device(I2C1_bus_handle, &ds1338_conf.conf, &ds1338_conf.handle);
    ESP_LOGI(TAG, "init ds1338 %s", ret == ESP_OK ? "success" : "failed");
    return ret;
}

esp_err_t i2c_ds1338_read(time_t *time){
    // struct tm timeinfo; // Removed unused variable
    esp_err_t ret;
    struct tm timeinfo;
    ret = i2c_master_transmit_receive( ds1338_conf.handle, 
                                       &ds1338_conf.addr,ds1338_conf.dl_addr,
                                       ds1338_conf.buff, ds1338_conf.dl_r, -1);
    ESP_LOGI(TAG, "DS1338 read data: %02x,%02X,%02X,%02X,%02X,%02X,%02X", 
            dds1338Buf[0],dds1338Buf[1],dds1338Buf[2],dds1338Buf[3],
            dds1338Buf[4],dds1338Buf[5],dds1338Buf[6]);

    uint8_t seconds = ((((dds1338Buf[0] >> 4) & 0x07) * 10) + (dds1338Buf[0] & 0x0F));
    uint8_t minutes = ((((dds1338Buf[1] >> 4) & 0x07) * 10) + (dds1338Buf[1] & 0x0F));
    uint8_t hours = ((((dds1338Buf[2] >> 4) & 0x03) * 10) + (dds1338Buf[2] & 0x0F));
    uint8_t date = ((((dds1338Buf[4] >> 4) & 0x03) * 10) + (dds1338Buf[4] & 0x0F));
    uint8_t month = ((((dds1338Buf[5] >> 4) & 0x01) * 10) + (dds1338Buf[5] & 0x0F));
    uint8_t full_year = ((((dds1338Buf[6] >> 4) & 0x0F) * 10) + (dds1338Buf[6] & 0x0F));
    
    
    timeinfo.tm_sec  = seconds;
    timeinfo.tm_min  = minutes;
    timeinfo.tm_hour = hours;
    timeinfo.tm_mday = date;
    timeinfo.tm_mon  = month;    // 'tm_mon' es de 0 a 11
    timeinfo.tm_year = full_year + 100; // 'tm_year' es desde 1900
    timeinfo.tm_isdst = 0;          // Dejar que 'mktime' determine si es horario de verano
    

    *time=mktime(&timeinfo);
    return ret;
}

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

    //ESP_LOGI(TAG, "ADC1015 read data: %02x,%02X", adc1015Buf[0],adc1015Buf[1]);
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
/*
esp_err_t i2c_ds1338_init(i2c_master_bus_handle_t bus_handle, const i2c_ds1338_config_t *ds1338_config, i2c_ds1338_handle_t *ds1338_handle)
{
    esp_err_t ret = ESP_OK;
    i2c_ds1338_handle_t out_handle;
    out_handle = (i2c_ds1338_handle_t) calloc(1, sizeof(i2c_ds1338_handle_t));
    
    ESP_GOTO_ON_FALSE(out_handle, ESP_ERR_NO_MEM, err, TAG, "no memory for i2c ds1338 device");

    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = ds1338_config->ds1338_dev.scl_speed_hz,
        .device_address = ds1338_config->ds1338_dev.device_address,
    };

    out_handle->buffer = (uint8_t*)calloc(1, ds1338_config->addr_wordlen + I2C_EEPROM_MAX_TRANS_UNIT);
    ESP_GOTO_ON_FALSE(out_handle->buffer, ESP_ERR_NO_MEM, err, TAG, "no memory for i2c eeprom device buffer");

    out_handle->addr_wordlen = ds1338_config->addr_wordlen;
    out_handle->write_time_ms = ds1338_config->write_time_ms;
    *eeprom_handle = out_handle;

    return ESP_OK;
 
    *ds1338_handle = dev;
    return ESP_OK;

    err:
    if (out_handle && out_handle->i2c_dev) {
        i2c_master_bus_rm_device(out_handle->i2c_dev);
    }
    free(out_handle);
    return ret;
}*/
/*
esp_err_t i2c_ds1338_write(i2c_eeprom_handle_t eeprom_handle, uint32_t address, const uint8_t *data, uint32_t size)
{
    ESP_RETURN_ON_FALSE(eeprom_handle, ESP_ERR_NO_MEM, TAG, "no mem for buffer");
    for (int i = 0; i < eeprom_handle->addr_wordlen; i++) {
        eeprom_handle->buffer[i] = (address & (0xff << ((eeprom_handle->addr_wordlen - 1 - i) * 8))) >> ((eeprom_handle->addr_wordlen - 1 - i) * 8);
    }
    memcpy(eeprom_handle->buffer + eeprom_handle->addr_wordlen, data, size);

    return i2c_master_transmit(eeprom_handle->i2c_dev, eeprom_handle->buffer, eeprom_handle->addr_wordlen + size, -1);
}

esp_err_t i2c_ds1338_read(i2c_eeprom_handle_t eeprom_handle, uint32_t address, uint8_t *data, uint32_t size)
{
    ESP_RETURN_ON_FALSE(eeprom_handle, ESP_ERR_NO_MEM, TAG, "no mem for buffer");
    for (int i = 0; i < eeprom_handle->addr_wordlen; i++) {
        eeprom_handle->buffer[i] = (address & (0xff << ((eeprom_handle->addr_wordlen - 1 - i) * 8))) >> ((eeprom_handle->addr_wordlen - 1 - i) * 8);
    }

    return i2c_master_transmit_receive(eeprom_handle->i2c_dev, eeprom_handle->buffer, eeprom_handle->addr_wordlen, data, size, -1);
}

void i2c_ds1338_wait_idle(i2c_eeprom_handle_t eeprom_handle)
{
    // This is time for EEPROM Self-Timed Write Cycle
    vTaskDelay(pdMS_TO_TICKS(eeprom_handle->write_time_ms));
}

*/