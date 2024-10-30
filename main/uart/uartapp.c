/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "uart/uartapp.h"

static const char *TAG = "uart_events";

// uart port number
#define uart_num 1

static QueueHandle_t uart1_queue;
QueueHandle_t uart_app_queue = NULL;


uint8_t dataToWrite[8] = {0x5A, 0XA5, 0X05, 0X82, 0x00, 0x00, 0x00, 0x00}; // Secuencia a escribir los 4 ultimos se llenan de acuerdo al registro y valor a escribir

uint8_t header[HEADER_LENGTH] = {0x5A, 0xA5};
uint8_t writeSuccessful[ACK_LENGTH] = {0x5A, 0xA5, 0x03, 0x82, 0x4F, 0x4B};
uint8_t buffer[READING_LENGTH];

int brillo, presion, presionAct, tiempo, humedad = 0;
bool fugas, running = 0;
int seqLength = READING_LENGTH;

void uart_app(void *pvParameters)
{
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
    // Install UART driver, and get the queue.
    uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart1_queue, 0);
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    // set pins for uart
    ESP_ERROR_CHECK(uart_set_pin(uart_num, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN));

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Ejemplo de configuracion inicial para los valores, inicializar donde corresponda y comentar aca
    brillo = 5;
    presion = 7;
    tiempo = 15;
    humedad = 4;
    ESP_LOGI(TAG, "Serial iniciado!");
    //initScreen();

    for (;;)
    {
        // read from port serial Dwin
        checkSerialDwin();
    }
}

// Rutina que esta verificando si se ha recibido una secuencia de la pantalla
void checkSerialDwin()
{
    struct uartDataIn datos;
    int len = 0;
    //uint8_t inByte; //vallejo
    uint8_t inBytes[10];
    uint8_t temp=0;

    uart_get_buffered_data_len(uart_num, (size_t *)&len);
    if (len > 0)
    /*
    {
       
        // Provisional: para implementar similar como en Arduino se lee un solo byte
        uart_read_bytes(uart_num, &inByte, 1, 1);
         ESP_LOGI(TAG, "Secuencia recibida: %02x, %c", inByte, inByte);
        //ESP_LOGI(TAG, "Read %d bytes: '%02x'", len, inByte);
        // len = uart_read_bytes(uart_num, datarx, len, 10 / portTICK_PERIOD_MS);
        // ESP_LOGI(TAG, "Read %d bytes: '%s'", len, datarx);
        saveData(inByte);
    }
    */
    {
        uart_read_bytes(uart_num, &inBytes, len, 1);
        ESP_LOGI(TAG, "Read %d bytes: %s", len, inBytes);
        //ESP_LOGI(TAG, "Read %d bytes: '%02x', '%02x', '%02x', '%02x', '%02x', '%02x'", len, inBytes[0], 
        //        inBytes[1], inBytes[2], inBytes[3], inBytes[4], inBytes[5]);
        switch(inBytes[0]){
            case 'P':
                temp=(inBytes[3]&0x0F)*10;
                temp+=inBytes[4]&0x0F;
                ESP_LOGI(TAG, "presion = %d", temp);
                datos.command='P';
                datos.value=temp;
                
                break;
            case 'S':
                datos.command='P';
                temp=(inBytes[3]&0x0F)*10;
                temp+=inBytes[4]&0x0F;
                datos.value=temp;

                //saveData(inBytes[1]);
                break;
            default:
                break;
        }
        xQueueSend(uart_app_queue, &datos, 0);

    }
    vTaskDelay(5);
}

// Metodo para almacenar y procesar la secuencia recibida
void saveData(uint8_t inByte)
{
    // Desplazar los elementos del buffer hacia la izquierda
    for (int i = 0; i < seqLength - 1; i++)
    {
        buffer[i] = buffer[i + 1];
    }

    // Añadir el nuevo byte al final del buffer
    buffer[seqLength - 1] = inByte;

    // Comprobar si el buffer coincide con el encabezado
    if (buffer[0] == header[0] && buffer[1] == header[1])
    {
        // Si coincide el encabezado, comprobar el resto de la secuencia
        if (checkSequence())
        {
            ESP_LOGI(TAG, "Comunicacion confirmada!");
            ESP_LOGI(TAG, "Brillo: %d, presion: %d, tiempo: %d, humedad: %d, fugas: %d, running: %d", brillo, presion, tiempo, humedad, fugas, running);
        }
    }
}

// Funcion para chequear que la secuencia sea la correcta
// y almacenar el valor si es una secuencia diferente a de confirmacion de
// escritura con informacion de un registro
bool checkSequence()
{
    ESP_LOGI(TAG, "Secuencia recibida: %02x", buffer[3]);
    if (buffer[3] == 0x82)
    { // Secuencia despues de escribir
        for (int i = 0; i < ACK_LENGTH; i++)
        {
            if (buffer[i] != writeSuccessful[i])
            {
                return false;
            }
        }
        seqLength = READING_LENGTH; // A la espera del envio de informacion asincrona de la perilla
        return true;
    }
    else if (buffer[3] == 0x83)
    {                                                  // Secuencia de lectura, con informacion de un registro
        uint16_t reg = (buffer[4] << 8) | buffer[5];   // Obtiene el numero del registro
        uint16_t value = (buffer[7] << 8) | buffer[8]; // Guarda el valor
        switch (reg)
        {
        case BRILLO_REG:
            brillo = value >> 8;
            break;
        case PRESION_REG:
            presion = value;
            break;
        case TIEMPO_REG:
            tiempo = value;
            break;
        case HUMEDAD_REG:
            humedad = value;
            break;
        case FUGAS_REG:
            fugas = value;
            break;
        case RUNNING:
            running = value;
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

/**
 * @brief Escribe en uno de los registros de la pantalla DWIN
 *
 * Este metodo escribe en el numero de registro "reg" el valor "value",
 * y espera por la secuencia serial que indica que la escritura se realizo.
 * De recibir esta secuencia devuelve "true" y en caso contrario "false".
 *
 * @param reg Número del registro que se pretende escribir en la pantalla DWIN.
 * @param value Valor que se pretende escribir en el registro.
 * @return bool El registro fue escrito correctamente.
 *
 * @note Por ejemplo para cambiar los iconos de la pantalla de fugas usar:
 *  writeDWIN(FUGAS_REG, F_TESTING);
 *  writeDWIN(FUGAS_REG, F_HAPPY);
 *  writeDWIN(FUGAS_REG, F_SAD);
 * Segun se requiera el icono de iniciando prueba, el feliz o triste.
 */
bool writeDWIN(unsigned int reg, uint8_t value)
{
    // Pone en la secuencia a enviar el numero de registro
    dataToWrite[4] = (reg >> 8) & 0xFF; // Desplazar 8 bits a la derecha y enmascarar los 8 bits altos
    dataToWrite[5] = reg & 0xFF;        // Enmascarar los 8 bits bajos
    // Pone en la secuencia a enviar el valor a grabar en el registro
    if (reg == BRILLO_REG)
    {                           // Si se va a cambiar el valor del brillo
        dataToWrite[6] = value; // es en la parte alta del registro
    }
    else
    {                           // Para los demas
        dataToWrite[7] = value; // es la parte baja
    }
    // Envia por el serial conectado a la pantalla
    uart_write_bytes(uart_num, (const char *)dataToWrite, sizeof(dataToWrite));

    seqLength = ACK_LENGTH; // Esperara la secuencia de confirmacion de escritura
    return true;
}

// Metodo que inicializa los registros de la pantalla
void initScreen()
{
    uint8_t initData[16] = {0x5A, 0xA5, 0x0D, 0x82, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    writeDWIN(BRILLO_REG, brillo);
    while (seqLength == ACK_LENGTH)
    { // Se espera confirmacion de escritura
        checkSerialDwin();
            
    }

    initData[7] = presion;
    initData[9] = presionAct;
    initData[11] = tiempo;
    initData[13] = running;
    initData[15] = humedad;

    uart_write_bytes(uart_num, (const char *)initData, sizeof(initData));
    ESP_LOGI(TAG, "send data");
    seqLength = ACK_LENGTH;
}