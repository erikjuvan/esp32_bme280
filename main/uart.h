#pragma once

#include "esp_err.h"
#include <stdint.h>

int       uart_transmit(const char* data, int len);
int       uart_receive(uint8_t* dest, int len);
esp_err_t init_uart();