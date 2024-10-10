#ifndef UARTAPP_H
#define UARTAPP_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"


//uart pins
#define TXD_PIN 9
#define RXD_PIN 10
#define RTS_PIN -1
#define CTS_PIN -1
//#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (128)
#define RD_BUF_SIZE (BUF_SIZE)


/*funciones*/
void init_uart(void);


#endif // UART_H

