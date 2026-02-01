// led.h

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812_control.h"

#define LED_COLOR_RED 0x004F00
#define LED_COLOR_GREEN 0x5F0000
#define LED_COLOR_BLUE 0x0000FF
#define LED_COLOR_YELLOW 0x5F5F00
#define LED_COLOR_OFF 0x000000

// Test mode colors for buttons and joystick
#define LED_COLOR_TEST_BUTTON_0 0xFF0000  // Red for button 0
#define LED_COLOR_TEST_BUTTON_1 0x00FF00  // Green for button 1
#define LED_COLOR_TEST_BUTTON_2 0x0000FF  // Blue for button 2
#define LED_COLOR_TEST_BUTTON_3 0xFFFF00  // Yellow for button 3
#define LED_COLOR_TEST_BUTTON_4 0xFF00FF  // Magenta for button 4
#define LED_COLOR_TEST_JOYSTICK_X 0x00FFFF  // Cyan for X-axis
#define LED_COLOR_TEST_JOYSTICK_Y 0xFF8000  // Orange for Y-axis

#define LED_CONNECTION 0
#define LED_COLOR_STATUS 1
#define LED_BATTERY 2

typedef enum {
    LED_OFF,
    LED_CONSTANT,
    LED_PULSE,
    LED_BLINK
}led_mode_t;

typedef struct {
    led_mode_t led_mode;
    TickType_t led_flash_period;
    uint32_t led_color;
    uint32_t led_color_alternate;
    uint32_t led_color_current;
} led_config_t;


// initiates the led task
void led_init(void);

// sets the mode for an LED
void led_set(int led_index, led_config_t config);

// sets the battery LED indication
void led_batt_set(uint8_t charge_now);

// sets the connection LED indication
void led_conn_set(uint8_t connected);

// sets the car color LED indication
void led_status_color_set(uint32_t color);

// sets the battery LED indication for control mode
void led_batt_control_mode_set(uint32_t color);

// test mode functions
void led_test_mode_button_pressed(int button_index);
void led_test_mode_button_released(void);
void led_test_mode_joystick_x_active(void);
void led_test_mode_joystick_y_active(void);
void led_test_mode_joystick_inactive(void);

#endif // LED_H
