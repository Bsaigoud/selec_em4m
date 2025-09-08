/** @file
 * @brief EVSE EM4M
 * @author GOPIKRISHNA S
 * @version 0.1
 * @date 2025-01-30
 * 
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

/** Please Refer this data to get data from EM4M */
/*
|==============================================================|
|   Address         Hex Address        Parameter               |
|==============================================================|
|   30000           0x0000          Voltage Phase V1N          |
|   30002           0x0002          Voltage Phase V2N          |
|   30004           0x0004          Voltage Phase V3N          |
|   30006           0x0006          Avg Voltage VLN            |
|   30008           0x0008          Voltage Phase V12          |
|   30010           0x000A          Voltage Phase V23          |
|   30012           0x000C          Voltage Phase V31          |
|   30014           0x000E          Avg Voltage VLL            |
|   30016           0x0010          Current I1                 |
|   30018           0x0012          Current I2                 |
|   30020           0x0014          Current I3                 |
|   30022           0x0016          Avg Current                |
|==============================================================|
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "driver/uart.h"
#include "esp_log.h"

#define DEBUG_ENABLE        (0)

#define TXD_PIN   17  // Define TX pin
#define RXD_PIN   16  // Define RX pin
#define UART_NUM  UART_NUM_1
#define BUF_SIZE  256

esp_err_t init_energy_meter(void);
float read_em4m_input_reg(uint16_t addr);
int16_t read_em4m_holding_reg(uint16_t addr);