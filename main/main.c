#include "bme280.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "spi.h"
#include "string.h"
#include "uart.h"
#include "user_bme280.h"

#define BLINK_GPIO GPIO_NUM_2

static void led_blink_advanced(int times, int freq)
{
    for (int i = 0; i < times; ++i) {
        const int delay = 1000 / freq;
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay((delay / 2) / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay((delay / 2) / portTICK_PERIOD_MS);
    }
    // delay at the end, to distinguish multiple errors
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void led_blink(int times)
{
    led_blink_advanced(times, 3);
}

void init_gpio()
{
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 0);
}

void app_main()
{
    init_gpio();

    // Visualize beginning of program
    led_blink_advanced(10, 10);

    init_uart();
    init_spi();
    init_bme280();

    bme280_run();

    //xTaskCreate(&bme280_run, "bme280_task", CONFIG_FREERTOS_IDLE_TASK_STACKSIZE, NULL, tskIDLE_PRIORITY, NULL);
    //xTaskCreate(&foo, "bme280_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    /*
	// Testing if CPU resets
	init_gpio();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	xTaskCreate(&foo, "bme280_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	*/
}
