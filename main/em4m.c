/**
 * @file evse_em4m.c    
 * @author GOPIKRISHNA S
 * @brief 
 * @version 0.1
 * @date 2025-01-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "em4m.h"

static const char *TAG = "Modbus RTU";
static const char *TAG1 = "";

// Function to calculate CRC16 for Modbus RTU
uint16_t modbus_crc16(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Function to parse float value from received Modbus response (MSRF Format)
float parse_modbus_float(uint8_t *data) {
    uint32_t raw_value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    float *float_ptr = (float *)&raw_value;
    return *float_ptr;
}

// Function to initialize UART
static void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "UART initialized for Modbus RTU");
}

esp_err_t init_energy_meter(void) {
    uart_init();
    return ESP_OK;
}

float read_em4m_input_reg(uint16_t addr)
{
    uint8_t rx_buffer[BUF_SIZE];
    uint8_t modbus_request[8];
    memset(rx_buffer, 0, BUF_SIZE);
    memset(modbus_request, 0, 8);
    float read_Value = 0.0;

     // Common Modbus request settings
    modbus_request[0] = 0x01;  // Slave ID
    modbus_request[1] = 0x04;  // Function Code (Read Input Registers)
    modbus_request[4] = 0x00;  // Number of Registers (High Byte)
    modbus_request[5] = 0x02;  // Number of Registers (Low Byte)

    // Set start address Dynamically
    modbus_request[2] = (addr >> 8) & 0xFF; // Start Address High Byte
    modbus_request[3] = addr & 0xFF;        // Start Address Low Byte

    // Calculate CRC
    uint16_t crc = modbus_crc16(modbus_request, 6);
    modbus_request[6] = crc & 0xFF;        // CRC Low Byte
    modbus_request[7] = (crc >> 8) & 0xFF; // CRC High Byte

#if DEBUG_ENABLE
    // Print sending packet in hex format
    printf("\nSent Modbus Request: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%02X ", modbus_request[i]);
    }
    printf("\n");
#endif
    // Send Modbus request
    uart_flush(UART_NUM);
    uart_write_bytes(UART_NUM, (const char *)modbus_request, sizeof(modbus_request));

    // Wait for response
    vTaskDelay(pdMS_TO_TICKS(100));

    int len = uart_read_bytes(UART_NUM, rx_buffer, BUF_SIZE, pdMS_TO_TICKS(200));
    if (len > 0)
    {
        #if DEBUG_ENABLE
        printf("\nReceived Modbus Response: ");
        for (int i = 0; i < len; i++)
        {
            printf("%02X ", rx_buffer[i]); // Print response in Hex format
        }
        printf("\n");
        #endif

        if (len >= 7)
        {                                                    // Ensure at least 7 bytes are received (Address, Function, Byte Count, Data, CRC)
            read_Value = parse_modbus_float(&rx_buffer[3]); // Extract float data from response
            printf("|\r\n");
            printf("|\tAt Address : 0x%04X\n", addr);
            printf("|\tValue in float(MSRF): %.3f\n", read_Value);
             // Print the parsed float value as raw hex bytes
            union {
                float f;
                uint8_t b[4];
            } float_bytes;
            float_bytes.f = read_Value;

            printf("|\tValue in HEX: <0x%02X%02X%02X%02X>\n",
            float_bytes.b[0], float_bytes.b[1], float_bytes.b[2], float_bytes.b[3]);

             // Print the parsed float value as 16-bit words
            // union {
            //     float f;
            //     uint16_t w[2];  // Two 16-bit words
            // } float_words;
            // float_words.f = read_Value;

            // printf("|\tVal Hex : <0x%04X0x%04X>\t\n",
            // float_words.w[1], float_words.w[0]); // Ensure correct byte order
        }
        else
        {
            ESP_LOGE(TAG1, "Invalid Modbus response length!");
        }
    }
    else
    {
        printf("|\tNo response at address 0x%04X!\r\n", addr);
    }

return read_Value;
}

int16_t read_em4m_holding_reg(uint16_t addr)
{
    uint8_t rx_buffer[BUF_SIZE];
    uint8_t modbus_request[8];
    memset(rx_buffer, 0, BUF_SIZE);
    memset(modbus_request, 0, 8);
    int16_t read_Value = 0; // Store 16-bit integer result

    // Modbus Request for Holding Registers (Function Code 0x03)
    modbus_request[0] = 0x01;  // Slave ID
    modbus_request[1] = 0x03;  // Function Code (Read Holding Registers)
    modbus_request[4] = 0x00;  // Number of Registers (High Byte)
    modbus_request[5] = 0x01;  // Number of Registers (Low Byte)

    // Set Start Address Dynamically
    modbus_request[2] = (addr >> 8) & 0xFF; // Start Address High Byte
    modbus_request[3] = addr & 0xFF;        // Start Address Low Byte

    // Calculate CRC
    uint16_t crc = modbus_crc16(modbus_request, 6);
    modbus_request[6] = crc & 0xFF;        // CRC Low Byte
    modbus_request[7] = (crc >> 8) & 0xFF; // CRC High Byte

    // Send Modbus Request
    uart_flush(UART_NUM);
    uart_write_bytes(UART_NUM, (const char *)modbus_request, sizeof(modbus_request));

    // Wait for Response
    vTaskDelay(pdMS_TO_TICKS(100));

    int len = uart_read_bytes(UART_NUM, rx_buffer, BUF_SIZE, pdMS_TO_TICKS(200));
    if (len > 0)
    {
        if (len >= 5)  // Ensure at least 5 bytes received (Address, Function, Byte Count, Data, CRC)
        {
            // Extract 16-bit integer value from response
            uint16_t register_value = (rx_buffer[3] << 8) | rx_buffer[4]; // Only one register

            // Convert to signed 16-bit integer
            read_Value = (int16_t)register_value; // If using unsigned, you can omit the cast

            // Print Integer Output
            printf("|\r\n");
            printf("|\tAt Address : 0x%04X\n", addr);
            printf("|\tValue in Integer: %d\n", read_Value);  // Signed integer
            printf("|\tValue in HEX: <0x%04X>\n", register_value);  // Print as HEX
        }
        else
        {
            ESP_LOGE(TAG1, "Invalid Modbus response length!");
        }
    }
    else
    {
        printf("|\tNo response at address 0x%04X!\r\n", addr);
    }

    return read_Value;
}
