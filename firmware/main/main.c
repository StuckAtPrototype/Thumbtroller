// main.c

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

// battery
#include "battery.h"

// adc
#include "adc_reader.h"

// interrupt on gpio
#include "gpio_interrupt.h"


#include <math.h>

// Constants
#define ADC_MIN 0
#define ADC_MAX 3280
#define ADC_CENTER ((ADC_MAX - ADC_MIN) / 2)
#define DEADZONE 50
#define MAX_SPEED 20
#define CURVE_FACTOR 1.1  // Adjust this to change the curve's steepness

// Function to map ADC value to -MAX_SPEED to MAX_SPEED range with deadzone and exponential curve
int map_joystick(int value) {
    // Apply deadzone
    if (abs(value - ADC_CENTER) < DEADZONE) {
        return 0;
    }

    // Normalize the value to a -1.0 to 1.0 range
    float normalized;
    if (value > ADC_CENTER) {
        normalized = (float)(value - ADC_CENTER - DEADZONE) / (ADC_MAX - ADC_CENTER - DEADZONE);
    } else {
        normalized = (float)(value - ADC_CENTER + DEADZONE) / (ADC_CENTER - ADC_MIN - DEADZONE);
    }

    // Apply exponential curve
    float curved = powf(fabsf(normalized), CURVE_FACTOR) * (normalized < 0 ? -1 : 1);

    // Map the curved value to the MAX_SPEED range
    return (int)(curved * MAX_SPEED);
}


void game_logic_loop(void *pvParameters)
{
    while (1){
        int adc_reading;
        uint8_t direction = 1;
        int speed = 0;
        int mixer = 0;

        uint32_t io_num;
        uint8_t data[5] = {0,1,0,1,1}; // speed wheel A, direction wheel A, speed wheel B, direction wheel B, duration

        // read in the thumbstick analog
        read_adc(ADC1_CHANNEL_3, &adc_reading);

        mixer = map_joystick(adc_reading);


        if(gpio_get_level(INTERRUPT_PIN_A) == 1){
            // going forward
            direction = 1;
            speed = 50;

        } else if (gpio_get_level(INTERRUPT_PIN_B)){
            // going backwards
            direction = 0;
            speed = -40;
        } else {
            // give some extra juice if we are not moving
            if(mixer > 0)
                mixer += 25;
            else if (mixer < 0)
                mixer -= 25;
        }

        int speed_a = speed + mixer;
        int speed_b = speed - mixer;

        data[0] = abs(speed_a);
        data[2] = abs(speed_b);

        if(speed_a > 0)
            data[1] = 1; // direction forward
        else
            data[1] = 0; // backwards

        if(speed_b > 0)
            data[3] = 1; // direction forward
        else
            data[3] = 0;

//        // wheel A
//        if((speed + mixer) > 0)
//            data[0] = speed + mixer;
//        else
//            data[0] = 0;
//
//        data[1] = direction;
//
//        // wheel B
//        if((speed - mixer) > 0)
//            data[2] = speed - mixer;
//        else
//            data[2] = 0;
//
//        data[3] = direction;

//        if(data[0] > 0){
//            data[0] += 10;
//        }

        ESP_LOGI("main", "mixer: %i, d[0] %u, d[1] %u, d[2] %u, d[3] %u",
                 mixer, data[0], data[1], data[2], data[3]);

        write_to_target_characteristic(data, 5);

//        for(;;) {
//            if(xQueueReceive(gpio_evt_queue, &io_num, 0)) {
//                printf("GPIO[%lu] interrupt occurred (rising edge)!\n", io_num);
//            }
//        }

        vTaskDelay(pdMS_TO_TICKS(50));

    }

    ESP_LOGE("main", "returning from game loop!");

}

void app_main(void)
{
    // initialize the leds
    led_init();

    // Turn on all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led(i, true);
    }

    // initilize interrupt on IO4 and IO5 (buttons) -- do we need this?
    configure_gpio_interrupt();

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

    xTaskCreate(game_logic_loop, "game_logic_loop", 2048, NULL, 5, NULL);


    while (1) {

//        int adc_reading;
//        float battery_voltage;
//
//        // Read from IO0
//        if (read_adc(ADC1_CHANNEL_0, &adc_reading) == ESP_OK) {
//            printf("IO0 ADC Reading: %d\n", adc_reading);
//        }
//
//        // Read from IO1
//        if (read_adc(ADC1_CHANNEL_1, &adc_reading) == ESP_OK) {
//            printf("IO1 ADC Reading: %d\n", adc_reading);
//        }
//
//        // Read battery voltage
//        if (read_battery_voltage(&battery_voltage) == ESP_OK) {
//            printf("Battery Voltage: %.3f V\n", battery_voltage);
//        }

//        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second

//        float voltage;
//        ret = battery_read_voltage(&voltage);
//        if (ret == ESP_OK) {
//            ESP_LOGI("main", "Battery Voltage: %.2f V", voltage);
//        } else {
//            ESP_LOGE("main", "Failed to read battery voltage");
//        }

//        if(voltage < 3.5){
//            set_led(0, false);
//            set_led(1, false);
//        } else if (voltage > 3.6){
//            set_led(0, true);
//            set_led(1, true);
//        }

        vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay for 2 seconds

    }
}
