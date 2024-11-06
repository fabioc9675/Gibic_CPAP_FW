#include "i2c/i2c_drv/i2c_common.h"

#define DEBUG

// Sensor Commands
typedef enum{
  /// Undefined dummy command.
  COMMAND_UNDEFINED                       = 0x0000,
  /// Start continous measurement                     \n
  /// Temperature compensation: Mass flow             \n
  /// Averaging: Average till read
  COMMAND_START_MEASURMENT_MF_AVERAGE     = 0x3603,
  /// Start continous measurement                     \n
  /// Temperature compensation: Mass flow             \n
  /// Averaging: None - Update rate 1ms
  COMMAND_START_MEASURMENT_MF_NONE        = 0x3608,
  /// Start continous measurement                     \n
  /// Temperature compensation: Differential pressure \n
  /// Averaging: Average till read
  COMMAND_START_MEASURMENT_DP_AVERAGE     = 0x3615,
  /// Start continous measurement                     \n
  /// Temperature compensation: Differential pressure \n
  /// Averaging: None - Update rate 1ms
  COMMAND_START_MEASURMENT_DP_NONE        = 0x361E,
  // Stop continuous measurement.
  COMMAND_STOP_CONTINOUS_MEASUREMENT      = 0x3FF9
}Command;

static const float scaleFactorTemperature = 200;

uint8_t sdp810Buf[9]={0};

i2c_sdp810_config_t sdp810_conf ={
    .conf.scl_speed_hz = I2C_FREQ_HZ,
    .conf.device_address = SDP810_ADDR,
    .conf.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    //.conf.flags.disable_ack_check = 1,
    .buff=sdp810Buf,
    .wt_ms = 1,
};

/**
 * local function to execute a command
 */
esp_err_t xReadMeasurementRawResults(int16_t *diffPressureTicks,
                                     int16_t *temperatureTicks,
                                     uint16_t *scaleFactor);
esp_err_t xExecuteCommand(Command cmd);
esp_err_t xCheckCrc(uint8_t *data, uint8_t size, uint8_t checksum);


/**
 * @brief Init Sdp810
 */
esp_err_t xSdp810Init(){
    esp_err_t ret;
    ret = i2c_master_bus_add_device(I2C1_bus_handle, &sdp810_conf.conf, &sdp810_conf.handle);
    return ret;
}

/**
 * @brief Starts the continous mesurement with the specified settings.
 */
esp_err_t xSdp810_StartContinousMeasurement(Sdp800TempComp tempComp,
                                            Sdp800Averaging averaging){

    esp_err_t ret;
    Command command = COMMAND_UNDEFINED;
    
    #ifdef DEBUG
    printf("tempcomp: %d\n",tempComp);
    printf("averaging: %d\n",averaging);
    #endif
    // determine command code
    switch(tempComp) {
        case SDP800_TEMPCOMP_MASS_FLOW:
        #ifdef DEBUG
        printf("SDP800_TEMPCOMP_MASS_FLOW\n");
        #endif
            switch(averaging){
                case SDP800_AVERAGING_TILL_READ:
                    #ifdef DEBUG
                    printf("SDP800_AVERAGING_TILL_READ\n");
                    #endif
                    command = COMMAND_START_MEASURMENT_MF_AVERAGE;
                    break;
        
                case SDP800_AVERAGING_NONE:
                    #ifdef DEBUG
                    printf("SDP800_AVERAGING_NONE\n");
                    #endif
                    command = COMMAND_START_MEASURMENT_MF_NONE;
                    break;
            }
            break;
    
        case SDP800_TEMPCOMP_DIFFERNTIAL_PRESSURE:
        #ifdef DEBUG
        printf("SDP800_TEMPCOMP_DIFFERNTIAL_PRESSURE\n");
        #endif
            switch(averaging) {
                case SDP800_AVERAGING_TILL_READ:
                    #ifdef DEBUG
                    printf("SDP800_AVERAGING_TILL_READ\n");
                    #endif
                    command = COMMAND_START_MEASURMENT_DP_AVERAGE;
                    break;
        
                case SDP800_AVERAGING_NONE:
                    #ifdef DEBUG
                    printf("SDP800_AVERAGING_NONE\n");
                    #endif
                    command = COMMAND_START_MEASURMENT_DP_NONE;
                    break;
            }
            break;
    }
  
    if(COMMAND_UNDEFINED != command) {
        #ifdef DEBUG
        printf("execute command: %04x\n",command);
        #endif
        ret = xExecuteCommand(command);
    } else {
        ret = ESP_ERR_INVALID_ARG;
    }
  
    return ret;
}




esp_err_t xSdp810_ReadMeasurementResults(float *diffPressure, float *temperature){

    esp_err_t ret;
    int16_t  diffPressureTicks;
    int16_t  temperatureTicks;
    uint16_t scaleFactorDiffPressure;
    
    ret = xReadMeasurementRawResults(&diffPressureTicks, &temperatureTicks,
                                    &scaleFactorDiffPressure);
    
    if (ret == ESP_OK){
        *diffPressure = (float)diffPressureTicks / (float)scaleFactorDiffPressure;
        *temperature = (float)temperatureTicks / scaleFactorTemperature;
    }
    //flow = PI*(0.027*0.027)*sqrt(2*9.81*(*diffPressure)/1.2);
    return ret;
}


/**
 * @brief Read the raw measurement results from the sensor.
 */
esp_err_t xReadMeasurementRawResults(int16_t *diffPressureTicks,
                                     int16_t *temperatureTicks,
                                     uint16_t *scaleFactor){

    esp_err_t ret;
    ret = i2c_master_receive(sdp810_conf.handle, sdp810_conf.buff, 9, sdp810_conf.wt_ms);
  
    if (ret == ESP_OK){
        //transaction success
        //let check the crc
        for (int i = 0; i < 3; i++){
            if (ret != ESP_OK){
                break;
            }else{
                ret = xCheckCrc(sdp810Buf+(i*3), 2, sdp810Buf[(i*3)+2]);
            } 
        }
        
        if(ret == ESP_OK) {
            *diffPressureTicks = (sdp810Buf[0] << 8) | sdp810Buf[1];
            *temperatureTicks = (sdp810Buf[3] << 8) | sdp810Buf[4];
            *scaleFactor = (sdp810Buf[6] << 8) | sdp810Buf[7];
        }else{
            ret = ESP_FAIL;    
            *diffPressureTicks = 0;
            *temperatureTicks = 0;
            *scaleFactor = 0;
        }
    }
    return ret;
}

/**
 * local function to execute a command
 */
esp_err_t xExecuteCommand(Command cmd){
    esp_err_t ret;
    sdp810Buf[0] = cmd >> 8;
    sdp810Buf[1] = cmd & 0xFF;
    ret = i2c_master_transmit(sdp810_conf.handle, 
                              sdp810Buf,3, 1);
    return ret;
}

/**
 * @brief Read a word from the sensor and check the CRC.
 */
esp_err_t xCheckCrc(uint8_t *data, uint8_t size, uint8_t checksum){
    uint8_t crc = 0xFF;
  
    // calculates 8-Bit checksum with given polynomial 0x31 (x^8 + x^5 + x^4 + 1)
    for(int i = 0; i < size; i++) {
        crc ^= (data[i]);
            for(uint8_t bit = 8; bit > 0; --bit) {
                (crc & 0x80) ? (crc = (crc << 1) ^ 0x31) : (crc = (crc << 1));
            }
    }     
    // verify checksum
  return (crc == checksum) ? ESP_OK : ESP_FAIL;
}

//27mm
//12mm

/*esp_err_t xSdp810SoftReset(void)
{
  esp_err_t ret = ESP_OK;
  
  // write a start condition
  I2c_StartCondition();

  // write the upper 8 bits of reset
  error = I2c_WriteByte(0x00);
  
  // write the lower 8 bits of reset
  if(ERROR_NONE == error) {
    error = I2c_WriteByte(0x06);
  }
  
  I2c_StopCondition();

  // wait 20 ms
  DelayMicroSeconds(20000); 

  return ret;
}
*/



// reset the sensor
/*
    Sdp800_SoftReset();

    error = Sdp800_StartContinousMeasurement(SDP800_TEMPCOMP_MASS_FLOW,
                                             SDP800_AVERAGING_TILL_READ);

    error = Sdp800_ReadMeasurementResults(&diffPressure, &temperature);
    if(error != ERROR_NONE) break;
    */