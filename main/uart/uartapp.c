/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "uart/uartapp.h"

static const char *TAG = "uart_events";


// uart port number
#define uart_num  0

static QueueHandle_t uart1_queue;

void uart_app(void *pvParameters)
{
    uint8_t datarx[16];
    int len = 0;
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart1_queue, 0);
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    //set pins for uart
    ESP_ERROR_CHECK(uart_set_pin(uart_num, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN));

    
    
    

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);

    //Set uart pattern detect function.
    //uart_enable_pattern_det_baud_intr(uart_num, '+', PATTERN_CHR_NUM, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    // uart_pattern_queue_reset(uart_num, 20);
   
    //disable uart pattern detect function.
    //uart_disable_pattern_det_intr(uart_num);

    //Create a task to handler UART event from ISR


    for(;;)
    {

        uart_get_buffered_data_len(uart_num, (size_t*)&len);

        //uart_write_bytes(uart_num, (const char *) "Hello", 5);
        //ESP_LOGI(TAG, "send data");
        
        //Read data from the UART
        
        if (len > 0) {
            ESP_LOGI(TAG, "Buffered data len: %d", len);
            len = uart_read_bytes(uart_num, datarx, len, 10 / portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "Read %d bytes: '%s'", len, datarx);
        }
     
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}