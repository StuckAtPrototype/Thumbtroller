#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

// BLE
#include "gap.h"
#include "gatt_svr.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "led.h"
#include "led_color_lib.h"
#include "ws2812_control.h"

// adc
#include "adc_reader.h"

// button interrupt
#include "button_isr.h"

// game logic
#include "game_logic.h"

struct led_state new_state1;

void app_main(void)
{
    // initialize the leds
    led_init();

    // set the initial sate of the LEDs
    led_batt_set(0);
    led_conn_set(0);
    led_status_color_set(LED_COLOR_BLUE);

    gpio_num_t button_pins[] = {GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_11, GPIO_NUM_22, GPIO_NUM_3};
    QueueHandle_t button_events = xQueueCreate(10, sizeof(gpio_num_t));

    button_isr_init(button_pins, 5, button_events);

    xTaskCreate(button_isr_debounce_task, "btn_debounce", 2048,
                NULL, 10, NULL);

    // BLE Setup -------------------
    nimble_port_init();
    ble_hs_cfg.sync_cb = sync_cb;
    ble_hs_cfg.reset_cb = reset_cb;
    gatt_svr_init();
    ble_svc_gap_device_name_set(device_name);
    nimble_port_freertos_init(host_task);

    // Initialize ADC
    esp_err_t result = adc_init();
    if (result != ESP_OK) {
        printf("ADC initialization failed\n");
        return;
    }

    game_init();

    // all the main logic is happening within game logic loop
    xTaskCreate(game_logic_loop, "game_logic_loop", 4096, NULL, 1, NULL);

    while (1) {


        vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay for 200 ms
    }
}