#include "uart.h"
#include "driver/uart.h"

#define UART_NUM UART_NUM_2

#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16

static const int UART_RX_BUF_SIZE = 512;
static const int UART_TX_BUF_SIZE = 512;

int uart_transmit(const char* data, int len)
{
    return uart_write_bytes(UART_NUM, data, len);
}

int uart_receive(uint8_t* dest, int len)
{
    return uart_read_bytes(UART_NUM, dest, len, 1000 / portTICK_RATE_MS);
}

esp_err_t init_uart()
{
    esp_err_t           ret;
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    ret = uart_param_config(UART_NUM, &uart_config);
    ESP_ERROR_CHECK(ret);
    ret = uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_ERROR_CHECK(ret);
    // We won't use a buffer for sending data.
    ret = uart_driver_install(UART_NUM, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, 0, NULL, 0);
    ESP_ERROR_CHECK(ret);

    return ret;
}