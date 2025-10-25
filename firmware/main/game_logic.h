#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "adc_reader.h"

// Constants for joystick mapping
#define ADC_MIN 0
#define ADC_MAX 3280
#define ADC_CENTER ((ADC_MAX - ADC_MIN) / 2)
#define DEADZONE 150
#define MAX_SPEED 80 // Maximum speed value for the car
#define CURVE_FACTOR 1.4  // Adjust this to change the curve's steepness

// Game control mode (exported to be accessible from main.c)
extern uint32_t game_control_mode;

// Function prototypes
void game_init(void);
void game_toggle_control_mode(void);
void game_change_car_color(void);
void game_logic_loop(void *pvParameters);
int map_joystick(int value, int max_value, float curve_factor);
int map_speed(int value, int max_value);
void reset_smoothing_variables(void);
void game_button_2_pressed(void);
void game_button_2_released(void);
void game_button_3_pressed(void);
void game_button_3_released(void);

// test mode functions
void game_test_mode_button_pressed(int button_index);
void game_test_mode_button_released(void);
void game_test_mode_joystick_check(int adc_x, int adc_y);
bool game_test_mode_is_button_pressed(void);

#endif // GAME_LOGIC_H