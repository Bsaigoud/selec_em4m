#include "em4m.h"

static const char *TAG = "EM4M";
static const char *TAG1 = "";

float read_val = 0.0;
uint16_t maxAddress = 0x0010;   // Set the maximum address here (HEX : 0x0000 to 0x0070 or INTEGER : 0 to 112)
uint16_t maxHoldAddress = 0x0011;
uint8_t i = 0;

void app_main() {

    esp_err_t err = init_energy_meter();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize energy meter: %s", esp_err_to_name(err));
        return;
    }
    else {
        ESP_LOGI(TAG, "Energy meter initialized");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for UART setup

    while (1) {

        ESP_LOGI(TAG1, "\r\n");
#if 1
        for (uint16_t addr = 0x0000; addr <= maxAddress; addr += 2) {

            if(addr == 0x0000) {
                printf("*********************************************************\r\n");
                printf("|\tRead from Input Register, Test %d Results\t|\r\n", ++i);
                printf("*********************************************************\r\n");
            }
            read_val = read_em4m_input_reg(addr);
            // printf("|\tAddress: 0x%04x, Value: %.2f\t|\r\n", addr, read_val);
        }

#else

        for (uint16_t addr = 0x0000; addr <= maxAddress; addr++) {
            if(addr == 0x0000) {
                printf("*********************************************************\r\n");
                printf("|\tRead from Holding Register, Test %d Results\t|\r\n", ++i);
                printf("*********************************************************\r\n");
            }
            uint16_t hold_val = read_em4m_holding_reg(addr);
            // printf("|\tAddress: 0x%04x, Value: %.2f\t|\r\n", addr, hold_val);
        }
#endif
        printf("*********************************************************\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Read every 5 seconds
    }
}