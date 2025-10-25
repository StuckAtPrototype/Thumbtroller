// led.c

#include "led.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_log.h"

#define TAG "LED"

// static function definitions
static void led_task(void *pvParameters);

// static variables
static struct led_state led_new_state;


/* static declaration of the shared configuration
 * LED0 == connection
 * LED1 == car color sync
 * LED2 == battery status
 */
static led_config_t led_config[3];

void led_init(void) {
    ws2812_control_init();
    // initiate the structures
    led_config[LED_BATTERY].led_color = LED_COLOR_GREEN;
    led_config[LED_CONNECTION].led_color = LED_COLOR_GREEN;
    led_config[LED_COLOR_STATUS].led_color = LED_COLOR_GREEN;

    BaseType_t ret = xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED task");
    } else {
        ESP_LOGI(TAG, "LED task created successfully");
    }
}

static void led_write_config(void){

}

void led_set(int led_index, led_config_t config) {
    if (led_index < 0 || led_index >= NUM_LEDS) {
        ESP_LOGE(TAG, "Invalid LED index");
        return;
    }

    // set the led configuration
    led_config[led_index] = config;

}

void led_batt_set(uint8_t charge_now){
    if(charge_now == 1){
        led_config[LED_BATTERY].led_color = LED_COLOR_RED;
    } else {
        led_config[LED_BATTERY].led_color = LED_COLOR_GREEN;
    }
}

void led_conn_set(uint8_t connected){
    if(connected == 1){
        led_config[LED_CONNECTION].led_color = LED_COLOR_BLUE;
        ESP_LOGI(TAG, "Connection LED set to BLUE (0x%08X)", LED_COLOR_BLUE);
    } else {
        led_config[LED_CONNECTION].led_color = LED_COLOR_OFF;
        ESP_LOGI(TAG, "Connection LED set to OFF");
    }
}

void led_status_color_set(uint32_t color){
    led_config[LED_COLOR_STATUS].led_color = color;
}

void led_batt_control_mode_set(uint32_t color){
    led_config[LED_BATTERY].led_color = color;
}

// Test mode functions
void led_test_mode_button_pressed(int button_index) {
    uint32_t color;
    switch(button_index) {
        case 0:
            color = LED_COLOR_TEST_BUTTON_0;
            break;
        case 1:
            color = LED_COLOR_TEST_BUTTON_1;
            break;
        case 2:
            color = LED_COLOR_TEST_BUTTON_2;
            break;
        case 3:
            color = LED_COLOR_TEST_BUTTON_3;
            break;
        case 4:
            color = LED_COLOR_TEST_BUTTON_4;
            break;
        default:
            color = LED_COLOR_OFF;
            break;
    }
    led_config[LED_CONNECTION].led_color = color;
}

void led_test_mode_button_released(void) {
    led_config[LED_CONNECTION].led_color = LED_COLOR_OFF;
}

void led_test_mode_joystick_x_active(void) {
    led_config[LED_CONNECTION].led_color = LED_COLOR_TEST_JOYSTICK_X;
}

void led_test_mode_joystick_y_active(void) {
    led_config[LED_CONNECTION].led_color = LED_COLOR_TEST_JOYSTICK_Y;
}

void led_test_mode_joystick_inactive(void) {
    led_config[LED_CONNECTION].led_color = LED_COLOR_OFF;
}

// this will alter the LED behavior over time
// for example pulse, blink, etc
// it will also clear state over time as necessary
static void led_task(void *pvParameters) {

    while(1){
        for(int i = 0; i < NUM_LEDS; i++){
            led_new_state.leds[i] = led_config[i].led_color;
        }
        

        ws2812_write_leds(led_new_state);

        vTaskDelay(pdMS_TO_TICKS(100));
    }

}