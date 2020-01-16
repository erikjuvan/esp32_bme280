#pragma once

#include "driver/spi_master.h"

extern spi_device_handle_t spi;

esp_err_t init_spi();
