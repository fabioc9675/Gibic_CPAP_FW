#include "i2c/i2c_drv/i2c_common.h"
#include <time.h>

static const char TAG[] = "ds1338";

uint8_t dds1338Buf[7]={0};

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

esp_err_t i2c_ds1338_init(){
    esp_err_t ret;
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