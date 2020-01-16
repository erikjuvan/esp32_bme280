#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_bme280.h"

#include "bme280.h"
#include "spi.h"
#include "uart.h"

static struct bme280_dev dev;

void user_delay_ms(uint32_t ms)
{
    // Return control or wait, for a period amount of milliseconds
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
    /*
     * The parameter dev_id can be used as a variable to select which Chip Select pin has
     * to be set low to activate the relevant device on the SPI bus
     */

    /*
     * Data on the bus should be like
     * |----------------+---------------------+-------------|
     * | MOSI           | MISO                | Chip Select |
     * |----------------+---------------------|-------------|
     * | (don't care)   | (don't care)        | HIGH        |
     * | (reg_addr)     | (don't care)        | LOW         |
     * | (don't care)   | (reg_data[0])       | LOW         |
     * | (....)         | (....)              | LOW         |
     * | (don't care)   | (reg_data[len - 1]) | LOW         |
     * | (don't care)   | (don't care)        | HIGH        |
     * |----------------+---------------------|-------------|
     */

    esp_err_t         ret; /* Return 0 for Success, non-zero for failure */
    spi_transaction_t t;

    memset(&t, 0, sizeof(t)); //Zero out the transaction

    t.length    = len * 8; // 8 bits = 1 byte (reg_addr).
    t.addr      = reg_addr;
    t.rx_buffer = reg_data; // Data
    t.user      = NULL;     // D/C needs to be set to 1

    ret = spi_device_polling_transmit(spi, &t); //Transmit!

    assert(ret == ESP_OK); //Should have had no issues.

    return ret;
}

int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
    /*
     * The parameter dev_id can be used as a variable to select which Chip Select pin has
     * to be set low to activate the relevant device on the SPI bus
     */

    /*
     * Data on the bus should be like
     * |---------------------+--------------+-------------|
     * | MOSI                | MISO         | Chip Select |
     * |---------------------+--------------|-------------|
     * | (don't care)        | (don't care) | HIGH        |
     * | (reg_addr)          | (don't care) | LOW         |
     * | (reg_data[0])       | (don't care) | LOW         |
     * | (....)              | (....)       | LOW         |
     * | (reg_data[len - 1]) | (don't care) | LOW         |
     * | (don't care)        | (don't care) | HIGH        |
     * |---------------------+--------------|-------------|
     */

    esp_err_t         ret; /* Return 0 for Success, non-zero for failure */
    spi_transaction_t t;

    if (len == 0)
        return 0; //no need to send anything

    memset(&t, 0, sizeof(t)); //Zero out the transaction

    t.length    = len * 8; // Len is in bytes, transaction length is in bits.
    t.addr      = reg_addr;
    t.tx_buffer = reg_data; // Data
    t.user      = (void*)1; // D/C needs to be set to 1

    ret = spi_device_polling_transmit(spi, &t); //Transmit!

    assert(ret == ESP_OK); //Should have had no issues.

    return ret;
}

int8_t init_bme280()
{
    int8_t rslt = BME280_OK;

    /* Sensor_0 interface over SPI with native chip select line */
    dev.dev_id   = 0;
    dev.intf     = BME280_SPI_INTF;
    dev.read     = user_spi_read;
    dev.write    = user_spi_write;
    dev.delay_ms = user_delay_ms;

    rslt = bme280_init(&dev);

    return rslt;
}

static void print_sensor_data(struct bme280_data* comp_data)
{
    char data[60] = {0};
#ifdef BME280_FLOAT_ENABLE
    snprintf(data, sizeof(data), "%0.2f, %0.2f, %0.2f\n", comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
    snprintf(data, sizeof(data), "%.2f *C, %.1f %%rH, %d mBar", comp_data->temperature / 100.0, comp_data->humidity / 1024.f, comp_data->pressure / 10000);
#endif
    uart_transmit(data, strlen(data));
}

void bme280_run()
{
    int8_t             rslt;
    uint8_t            settings_sel;
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    dev.settings.osr_h  = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p  = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t  = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &dev);

    /* Continuously stream sensor data */
    while (1) {
        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);

        /* Wait for the measurement to complete and print data @25Hz */
        dev.delay_ms(1000);
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

        print_sensor_data(&comp_data);
    }
}
