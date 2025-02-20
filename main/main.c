#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt/mqtt.h"

#include "uSD/usdApp.h"
#include "i2c/i2c_app.h"
#include "bldc/bldc_servo.h"
#include "uart/uartapp.h"
#include "control.h"

//#define CARACTERIZACION

/**
 * flags del sistema
 */
uint8_t flProcSen = 0; //flag para procesar sensores se stea en 1 cuando hay datos nuevos

/**
 * varaibles golbales para el sisitema
 */
uint8_t setPointPresion = 0;

//todo: verificar si el timestamp esta en segundos o milisegundos.

uint16_t bldc_sp = 100;
uint8_t flag_bldc = 0;
uint8_t spbldctemp = 0;

/**
 * temporal para enviar a la pantalla
 */
uint8_t cnt=10;

#ifdef CARACTERIZACION
//para caraterisar sensor de presion
uint8_t flag_temp = 0;
uint16_t segundos_30 = 0;
uint16_t temp_bldc = 0;
#endif

/*
 *Variable para manejar los estados del sistema
 */
typedef enum
{
    idle=0,
    initfilesd,
    initCpap,
    endCpap
} MS_STATES;

MS_STATES msEstados = idle;




//TaskHandle_t bldcTaskHandle = NULL; // Identificador para la tarea i2c_app
void app_main(void)
{
    
    struct Datos_usd datos_usd;
 
    datos_usd.bldc = 0;

    //creamos cola para envio a la sd
    sd_App_queue = xQueueCreate(10, sizeof(struct Datos_usd));
    i2c_App_queue = xQueueCreate(10, sizeof(struct Datos_I2c));
    bldc_App_queue = xQueueCreate(10, sizeof(uint16_t));
    uart_app_queue = xQueueCreate(10, sizeof(struct uartDataIn));
    uart_app_queue_rx = xQueueCreate(10, sizeof(struct ToUartData ));
   
    //dato para bldc en 0
    bldc_sp = 0xFFFE;
    xQueueSend(bldc_App_queue, &bldc_sp, 0);
        
    //cramos tareas
    xTaskCreate(i2c_app, "i2c_app", 4096, NULL, 10, NULL);
    xTaskCreate(bldc_servo_app, "bldc_servo_app", 4096, NULL, 10, NULL);
    xTaskCreate(sd_App, "sd_App", 4096, (void *)&init_time, 10, NULL);
    xTaskCreate(uart_app, "uart_app", 4096, NULL, 10, NULL);
    
    //xQueueSend(bldc_App_queue, &bldc_sp, 0);
    init_time *= 1000;
    for(;;)
    {   
        while(uxQueueMessagesWaiting(i2c_App_queue) > 0){ //cola con datos de i2c

            /**
             * todo: la cola de la sd debe complementarse con datos de los 
             * algoritmos de deteccion de apnea. esta parte es temporal
             * aunque podria reutilizarse solo para encapsular los datos
             * para el algoritmo de control
             */
            
            struct Datos_I2c datos_i2c;
            xQueueReceive(i2c_App_queue, &datos_i2c, 0);
            datos_usd.timestamp = esp_log_timestamp() + init_time;
            datos_usd.presion = datos_i2c.presion;
            datos_usd.flujo = datos_i2c.flujo;
                //printf("presion %0.2f\n",datos_i2c.presion);

                /**
                 *esto es temporal   
                 */
                if(!cnt--){
                    struct ToUartData touartdata;
                    cnt=10;
                    touartdata.command = UPresion;
                    touartdata.value = (int8_t)datos_i2c.presion;
                    xQueueSend(uart_app_queue_rx, &touartdata,0);
                }
            datos_usd.tempflujo = datos_i2c.tempFlujo;
            xQueueSend(sd_App_queue, &datos_usd, 0);
            ESP_LOGI("MAIN", "presion: %0.2f, flujo: %0.2f", datos_usd.presion, datos_usd.flujo);
            flProcSen = 1;

        }

        while(uxQueueMessagesWaiting(uart_app_queue) > 0){
            struct uartDataIn datos;
            
            xQueueReceive(uart_app_queue, &datos, 0);
            printf("command: %c\n", datos.command);
                printf("value: %d\n", datos.value);

            switch (datos.command)
            {
            case 'P': //presion objetivo
                setPointPresion = datos.value;
                
            
                //printf("en case p con command: %c y data: %d\n", datos.command, datos.value);
                

                //escalamos la presion
                //spbldctemp = datos.value;
                /*
                printf("spbldctemp: %d\n", spbldctemp);
                bldc_sp = (spbldctemp-3)* 55;
                if (flag_bldc){
                    datos_usd.bldc = datos.value;
                    xQueueSend(bldc_App_queue, &bldc_sp, 0);

                }else{
                    datos_usd.bldc = 0;
                }
                */
                break;

            case 'S':
                if(datos.value==1){ // inicia tratamiento 

                    #ifdef CARACTERIZACION
                    //para caraterisar sensor de presion
                    flag_temp = 1;
                    #else
                    if (setPointPresion &&  msEstados == idle){
                        msEstados = initfilesd;
                        spbldctemp = setPointPresion;
                    }
                    // estructurar esta parte para que me juegue con otra maquina de estados
                    /*
                    flag_bldc = 1;
                    bldc_sp = (spbldctemp-3)* 55;
                    //printf("bldc_sp: %d\n", bldc_sp);
                    xQueueReset(bldc_App_queue);
                    xQueueSend(bldc_App_queue, &bldc_sp, 0);
                    datos_usd.bldc = spbldctemp;
                    //xTaskCreate(bldc_servo_app, "bldc_servo_app", 4096, NULL, 10, NULL);
                    */
                    #endif
                    
                }else if(datos.value==0){
                    /**
                     * @todo que hacer cuando terminamos?
                     */
                    setPointPresion = 0;
                    msEstados = endCpap;
                    /*
                    flag_bldc = 0;
                                        bldc_sp = 0xFFFE;
                    xQueueSend(bldc_App_queue, &bldc_sp, 0);
                    datos_usd.bldc = 0;
                    bldc_sp = 0;
                    */
                }   
                break;

            default:
                break;
            }
            
        }
        #ifdef CARACTERIZACION
        if(flag_temp){
            segundos_30++;
            if(segundos_30 == 1999){
                segundos_30 = 0;
                temp_bldc += 50;
                datos_usd.bldc = temp_bldc/10;
                if (temp_bldc > 1000){
                    temp_bldc = 0;
                    flag_temp = 0;

                }else{
                    ESP_LOGI("MAIN", "actualizar");
                    xQueueSend(bldc_App_queue, &temp_bldc, 0);
                }
               
            }
        }
        #endif
        //ESP_LOGI("MAIN", "DORMIR");
        //vTaskDelay(50 / portTICK_PERIOD_MS);

        //aca vendria la aquina de estados para el proceso del sistema
        switch (msEstados){
            case idle:
            //esperar comando
                break;
            
            case initfilesd:
                msEstados = initCpap;//iniciar proceso
                break;
            
            case initCpap:
                //procemos algoritmo de control con datos nuevos
                if (flProcSen){
                    flProcSen = 0;
                    bldc_sp = controller(setPointPresion, datos_usd.presion);
                    //ESP_LOGI("MAIN", "bldc_sp: %d", bldc_sp);
                    xQueueSend(bldc_App_queue, &bldc_sp, 0);
                    //procesar datos
                    //enviar a la pantalla
                    //enviar a la sd
                }
            //proceso en ejecucion
                break;
            case endCpap:
                /**
                 * @todo: que hacer cuando terminamos?
                 */
                bldc_sp = 0;
                xQueueSend(bldc_App_queue, &bldc_sp, 0);
                msEstados = idle;
                break;
            //proceso finalizado
            default:
                break;
        }



        vTaskDelay(1);
    }
}
//esp_log_timestamp(void)

/*
Para pruebas de mqtt 

printf("Run!\n");
mqtt_initVars("NATALIA", "1128418683", "mqtt://3.90.24.183", 8924);


for(;;)
    {   
     MSmqtt();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


*/