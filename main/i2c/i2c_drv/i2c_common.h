#ifndef I2C_COMMON_H
#define I2C_COMMON_H

#include <string.h>
#include <stdio.h>
#include <time.h>
#include "sdkconfig.h"
#include "esp_types.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

/*
 * I2C common functions and definitions
 * GPIO used for I2C1 and I2C2
 * frquency of I2C1 and I2C2
 */
#define SCL1           15    // GPIO number for I2C1 master clock
#define SDA1           16    // GPIO number for I2C1 master data
#define I2C_DEV1       I2C_NUM_0  // I2C port number for master dev
#define SCL2           15    // GPIO number for I2C1 master clock
#define SDA2           16    // GPIO number for I2C1 master data
#define I2C_DEV2       I2C_NUM_1  // I2C port number for master dev
#define I2C_FREQ_HZ    400000     // I2C master clock frequency
#define I2C_TIMEOUT_MS       5

/**
 * Addresses of I2C devices.
 */
#define RTC_ADDR    0x68   // DS1338_ADD RTC 400Khz
#define ADC_ADDR    0x48   // ADC1015_ADD ADC 400Khz

/*
 *externals variables for I2C1
 */
extern i2c_master_bus_config_t i2c1_bus_conf;
extern i2c_master_bus_handle_t I2C1_bus_handle;

/** 
 *@brief init I2C1
 *@param None
 *@return ESP_OK: Init success. ESP_FAIL: Not success.
 */
esp_err_t I2C1_init(void);



/**************************************************************************
 * For rtc DS1338
 */

/**
 * typedef for DS1338 config and handle
 */
typedef struct {
    i2c_device_config_t conf;         /*!< conf device for eeprom device */
    i2c_master_dev_handle_t handle;   /*!< I2C device handle */
    uint8_t addr;                     /*!< read address */
    uint8_t dl_addr;                  /*!< len add to read */
    uint8_t dl_r;                     /*!< len buf read */
    uint8_t *buff;                    /*!< buffer for r/w */
    uint8_t wt_ms;                    /*!< timeout for r/w */
} i2c_ds1338_config_t;

/**
 * function for DS1338
 */

/**
 * @brief Init an ds1338.
 * @param None
 * @return ESP_OK: Init success. ESP_FAIL: Not success.
 */
esp_err_t i2c_ds1338_init();

/**
 * @brief Read time from DS1338
 * @param[out] time time_t pointer
 * @return ESP_OK: Read success. Otherwise failed, please check I2C function fail reason.
 */
esp_err_t i2c_ds1338_read(time_t *time);



/**************************************************************************
 * For adc ADS1015
 */

/**
 * typedef for ADS1015 config and handle
 */
typedef struct {
    i2c_device_config_t conf;         /*!< conf device for eeprom device */
    i2c_master_dev_handle_t handle;   /*!< I2C device handle */
    uint8_t p_addr;                   /*!< pointer address */
    uint8_t dl_p_addr;                 /*!< len add to read */
    uint8_t dl_r;                     /*!< len buf read */
    uint8_t *buff;                    /*!< buffer for r/w */
    uint8_t wt_ms;                    /*!< timeout for r/w */
} i2c_adc1015_config_t;

/**
 * @brief Init ads1015.
 * @param None
 * @param[in] eeprom_config Configuration of EEPROM
 * @param[out] eeprom_handle Handle of EEPROM
 * @return ESP_OK: Init success. ESP_FAIL: Not success.
 */
esp_err_t i2c_adc1015_init();

/**
 * @brief Get data from ADS1015
 * @param[in] ch Channel of ADS1015
 * @param[out] data Data read from ADS1015 register 1
 * @return ESP_OK: Read success. Otherwise failed, please check I2C function fail reason.
 */
esp_err_t i2c_adc1015_get_ch(uint8_t ch, uint16_t *data);

/**
 * @brief Read data from ADS1015
 * @param[out] data Data read from ADS1015 register 0
 * @return ESP_OK: Read success. Otherwise failed, please check I2C function fail reason.
 * @note This function is used after i2c_adc1015_get_ch
 */
esp_err_t i2c_adc1015_read_ch(uint16_t *data);



/**************************************************************************
 * For SDP810
 */

/**
 * typedef for SDP810 config and handle
 */
typedef struct {
    i2c_device_config_t conf;         /*!< conf device for eeprom device */
    i2c_master_dev_handle_t handle;   /*!< I2C device handle */
    uint8_t p_addr;                   /*!< pointer address */
    uint8_t dl_p_addr;                 /*!< len add to read */
    uint8_t dl_r;                     /*!< len buf read */
    uint8_t *buff;                    /*!< buffer for r/w */
    uint8_t wt_ms;                    /*!< timeout for r/w */
} i2c_sdp810_config_t;


/**
 * @brief Write data to EEPROM
 *
 * @param[in] eeprom_handle EEPROM handle
 * @param[in] address Block address inside EEPROM
 * @param[in] data Data to write
 * @param[in] size Data write size
 * @return ESP_OK: Write success. Otherwise failed, please check I2C function fail reason.
 */
//esp_err_t i2c_eeprom_write(i2c_eeprom_handle_t eeprom_handle, uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief Read data from EEPROM
 *
 * @param eeprom_handle EEPROM handle
 * @param address Block address inside EEPROM
 * @param data Data read from EEPROM
 * @param size Data read size
 * @return ESP_OK: Read success. Otherwise failed, please check I2C function fail reason.
 */
//esp_err_t i2c_eeprom_read(i2c_eeprom_handle_t eeprom_handle, uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief Wait eeprom finish. Typically 5ms
 *
 * @param eeprom_handle EEPROM handle
 */
//void i2c_eeprom_wait_idle(i2c_eeprom_handle_t eeprom_handle);





#define SENSIRION_ADDR              0x40   // I2C address of another sensor
//#define ADC_ADDR

//extern 


#endif // I2C_COMMON_H


/*
 * For rtc ds1338
 */

/*typedef struct
{
  // Segundos
  uint8_t UNI_SEG :4;
  uint8_t DEC_SEG :3;
  uint8_t CH      :1;
  // Minutos
  uint8_t UNI_MIN :4;
  uint8_t DEC_MIN :3;
  uint8_t         :1;
  // Horas
  union 
  {
    struct 
    {
      // Horas 24
      uint8_t UNI_HOR :4;
      uint8_t DEC_HOR :2;
      uint8_t _12_24  :1;
      uint8_t        :1;
    }_24;
    struct 
    {
      // Horas 12
      uint8_t UNI_HOR :4;
      uint8_t DEC_HOR :1;
      uint8_t AM_PM   :1;
      uint8_t        :2;
    }_12;
  }_HORA;
  // Día de la semana
  unsigned char DIASEM  :3;
  unsigned char         :5;
  // Día del mes
  unsigned char UNI_DIA :4;
  unsigned char DEC_DIA :2;
  unsigned char         :2;
  // Mes
  unsigned char UNI_MES :4;
  unsigned char DEC_MES :1;
  unsigned char         :3;
  // Año
  unsigned char UNI_YEA :4;
  unsigned char DEC_YEA :4;
}_STR_RTC;*/