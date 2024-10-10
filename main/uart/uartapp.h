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
#define TXD_PIN 10
#define RXD_PIN 11
#define RTS_PIN (UART_PIN_NO_CHANGE)
#define CTS_PIN (UART_PIN_NO_CHANGE)
//#define EX_UART_NUM UART_NUM_0
#define BUF_SIZE (128)
//#define RD_BUF_SIZE (BUF_SIZE)

/*funciones*/
void uart_app(void *pvParameters);

#endif // UARTAPP_H

