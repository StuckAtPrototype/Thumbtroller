#include "game_logic.h"
#include "gap.h"
#include "esp_random.h"
#include "led.h"

// Initialize game control mode
uint32_t game_control_mode = 0;

uint8_t game_car_rgb[3] = {0xff,0xff,0xff};

// Test mode state tracking
static bool test_mode_button_pressed = false;
static int test_mode_active_button = -1;

// Smoothing variables for game_control_mode 0
static int prev_mixer_x = 0;
static int prev_mixer_y = 0;
static int adc_history_x[3] = {0, 0, 0};
static int adc_history_y[3] = {0, 0, 0};
static int history_index = 0;

// Button state tracking for game_control_mode 1
static bool button_2_pressed = false;  // 50% reverse
static bool button_3_pressed = false;  // full forward

// Function to smooth ADC readings using a simple moving average
int smooth_adc_reading(int new_reading, int* history, int* index) {
    history[*index] = new_reading;
    *index = (*index + 1) % 3;
    
    // Calculate average of last 3 readings
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += history[i];
    }
    return sum / 3;
}

// Function to apply exponential smoothing to mixer values
int smooth_mixer_value(int new_value, int prev_value, float smoothing_factor) {
    return (int)(prev_value * smoothing_factor + new_value * (1.0f - smoothing_factor));
}

// Function to map ADC value for speed with more responsive lower end
int map_speed(int value, int max_value) {
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

    // Use a balanced curve that reaches max speed at 90% joystick travel
    // This means at 0.85 normalized value, we want max output
    float curve_factor = 0.6f;  // More balanced curve factor for better low-end resolution
    float curved = powf(fabsf(normalized), curve_factor) * (normalized < 0 ? -1 : 1);
    
    // Scale so that 0.85 input gives max output
    curved = curved / 0.85f;
    if (fabsf(curved) > 1.0f) curved = curved > 0 ? 1.0f : -1.0f;

    // Map the curved value to the MAX_SPEED range
    return (int)(curved * max_value);
}

// Function to map ADC value to -MAX_SPEED to MAX_SPEED range with deadzone and exponential curve
int map_joystick(int value, int max_value, float curve_factor) {
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
    float curved = powf(fabsf(normalized), curve_factor) * (normalized < 0 ? -1 : 1);

    // Map the curved value to the MAX_SPEED range
    return (int)(curved * max_value);
}


// Function to reset smoothing variables when switching modes
void reset_smoothing_variables(void) {
    prev_mixer_x = 0;
    prev_mixer_y = 0;
    for (int i = 0; i < 3; i++) {
        adc_history_x[i] = 0;
        adc_history_y[i] = 0;
    }
    history_index = 0;
}

// Button handlers for game_control_mode 1
void game_button_2_pressed(void) {
    button_2_pressed = true;
    ESP_LOGI("game_logic", "Button 2 pressed - 50%% reverse");
}

void game_button_2_released(void) {
    button_2_pressed = false;
    ESP_LOGI("game_logic", "Button 2 released");
}

void game_button_3_pressed(void) {
    button_3_pressed = true;
    ESP_LOGI("game_logic", "Button 3 pressed - full forward");
}

void game_button_3_released(void) {
    button_3_pressed = false;
    ESP_LOGI("game_logic", "Button 3 released");
}

void game_toggle_control_mode(void){
    if(game_control_mode == 0) {
        game_control_mode = 1;
        // Set battery LED to YELLOW for mode 1 (other mode)
        led_batt_control_mode_set(LED_COLOR_YELLOW);
    } else {
        game_control_mode = 0;
        // Set battery LED to GREEN for mode 0 (initial mode)
        led_batt_control_mode_set(LED_COLOR_GREEN);
    }
    // Reset smoothing variables when switching modes
    reset_smoothing_variables();
}



void game_change_car_color(void){

    game_car_rgb[0] = esp_random() % 251;
    game_car_rgb[1] = esp_random() % 251;
    game_car_rgb[2] = esp_random() % 251;

    // Set the status LED to show the car color
    uint32_t game_car_lights = ((uint32_t)game_car_rgb[1] << 16) | ((uint32_t)game_car_rgb[0] << 8) | game_car_rgb[2];
    led_status_color_set(game_car_lights);

}

void game_init(void){
    // Reset smoothing variables on initialization
    reset_smoothing_variables();
    
    // Set battery LED color based on initial control mode
    if(game_control_mode == 0) {
        led_batt_control_mode_set(LED_COLOR_GREEN);
    } else {
        led_batt_control_mode_set(LED_COLOR_YELLOW);
    }
    
    // Initialize status LED with car color
    uint32_t game_car_lights = ((uint32_t)game_car_rgb[1] << 16) | ((uint32_t)game_car_rgb[0] << 8) | game_car_rgb[2];
    led_status_color_set(game_car_lights);

}

// Main game logic loop task
void game_logic_loop(void *pvParameters)
{
    while (1){
        int adc_reading_x;
        int adc_reading_y;
        uint8_t direction = 1;
        int speed = 0;
        int mixer_x = 0;
        int mixer_y = 0;
        int speed_a = 0;
        int speed_b = 0;
        int curve_max = 0;

        uint32_t io_num;

        // speed wheel A, direction wheel A, speed wheel B, direction wheel B, duration ( last 3 bytes are reserved for future use )
        uint8_t data[8] = {0,1,0,1,2, 0x00, 0x00, 0x00};

        // Read actual button states directly from GPIO
        button_2_pressed = (gpio_get_level(GPIO_NUM_11) == 1);  // Button 2
        button_3_pressed = (gpio_get_level(GPIO_NUM_22) == 1);  // Button 3
        
        // read in the thumbstick analog
        read_adc(THUMB_ADC_CHANNEL_1, &adc_reading_y);  // Y-axis (ADC1_CHANNEL_3)
        read_adc(THUMB_ADC_CHANNEL_0, &adc_reading_x);  // X-axis (ADC1_CHANNEL_0)

        // Check if we're in test mode (not connected to car)
        // Use BLE connection check (even if characteristic discovery isn't complete)
        if (!gap_ble_connected()) {
            // Test mode: check joystick input and show LED colors
            game_test_mode_joystick_check(adc_reading_x, adc_reading_y);
            
            // Skip normal game logic in test mode
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        } else {
            // Connected mode: ensure test mode state is reset
            if (test_mode_button_pressed) {
                test_mode_button_pressed = false;
                test_mode_active_button = -1;
            }
        }

        if(game_control_mode == 0){
            // Smooth the ADC readings to reduce noise
            int smoothed_x = smooth_adc_reading(adc_reading_x, adc_history_x, &history_index);
            int smoothed_y = smooth_adc_reading(adc_reading_y, adc_history_y, &history_index);
            
            // Use specialized speed mapping for more responsive lower end
            int raw_mixer_x = map_speed(smoothed_x, MAX_SPEED);
            // Progressive turning: less aggressive at low speeds, more aggressive at high speeds
            // Use a more gradual curve that starts low and increases with speed
            float speed_factor = (float)abs(raw_mixer_x) / MAX_SPEED;  // 0.0 to 1.0
            curve_max = (int)((speed_factor * speed_factor * 28) + 15);  // Quadratic growth: 10-50 range
            if (curve_max > 60 ) curve_max = 60;  // Higher cap for high-speed turning
            int raw_mixer_y = map_joystick(smoothed_y, curve_max, 1.1);  // Higher curve factor for more sensitivity
            
            // Apply exponential smoothing to mixer values for smoother control
            // Use more aggressive smoothing to prevent jumps
            mixer_x = smooth_mixer_value(raw_mixer_x, prev_mixer_x, 0.85f);
            mixer_y = smooth_mixer_value(raw_mixer_y, prev_mixer_y, 0.85f);
            
            // Progressive turning limits: more aggressive at high speeds
            int max_turn = (int)(15 + (speed_factor * 30));  // 15-45 range based on speed
            if (mixer_y > max_turn) mixer_y = max_turn;
            if (mixer_y < -max_turn) mixer_y = -max_turn;
            
            // Store current values for next iteration
            prev_mixer_x = mixer_x;
            prev_mixer_y = mixer_y;

            // joystick only control mixer "vertical"
            if(mixer_x < 0){
                // going forward
                speed = abs(mixer_x) + 6;  // Hack: add 6 to reach max speed of 80
                direction = 1;
                speed_a = speed + mixer_y;
                speed_b = speed - mixer_y;
            } else {
                speed = abs(mixer_x) + 6;  // Hack: add 6 to reach max speed of 80
                direction = 0;
                speed_a = speed + mixer_y;
                speed_b = speed - mixer_y;
            }
        } else if (game_control_mode == 1){
            // Joystick controls turning (X-axis for horizontal movement)
            int smoothed_x = smooth_adc_reading(adc_reading_x, adc_history_x, &history_index);
            curve_max = 50;  // Higher turning sensitivity for mode 1
            mixer_y = map_joystick(smoothed_x, curve_max, 1.6);
            
            // Apply smoothing to turning
            mixer_y = smooth_mixer_value(mixer_y, prev_mixer_y, 0.85f);
            prev_mixer_y = mixer_y;
            
            // Button-based speed control for mode 1
            if (button_3_pressed) {
                // Button 3: Full speed forward
                speed = MAX_SPEED;
                direction = 1;
                speed_a = speed + mixer_y;
                speed_b = speed - mixer_y;
            } else if (button_2_pressed) {
                // Button 2: 80% speed reverse
                speed = MAX_SPEED * 0.8;  // 80% of max speed
                direction = 0;
                speed_a = speed + mixer_y;
                speed_b = speed - mixer_y;
            } else {
                // No buttons pressed: spin in place using joystick
                // For spinning, we need opposite wheel speeds with higher speed
                // int spin_speed = mixer_y * 2;  // Double the speed for more effective spinning
                // speed_a = spin_speed;   // One wheel speed
                // speed_b = -spin_speed;  // Opposite wheel speed
                // direction = 1;  // Default direction
                
                // I really don't like how the spin in place feels, feel free to uncomment this and try it out
            }
        } else {
            // Other control modes would go here
        }

        // sanitize
        if(speed_a > 100)
            speed_a = 100;
        if(speed_b > 100)
            speed_b = 100;

        // Handle direction for each wheel separately when spinning
        if (game_control_mode == 1 && !button_2_pressed && !button_3_pressed) {
            // Spinning mode: each wheel can have different directions
            data[0] = abs(speed_a);
            data[1] = speed_a >= 0 ? 1 : 0;  // Forward if positive, reverse if negative
            
            data[2] = abs(speed_b);
            data[3] = speed_b >= 0 ? 1 : 0;  // Forward if positive, reverse if negative
        } else {
            // Normal mode: both wheels use same direction
            data[0] = abs(speed_a);
            data[1] = direction;

            data[2] = abs(speed_b);
            data[3] = direction;
        }

        ESP_LOGD("game_logic", "mixer_x, %i, mixer_y %i, d[0] %u, d[1] %u, d[2] %u, d[3] %u",
                 mixer_x, mixer_y, data[0], data[1], data[2], data[3]);

        // read in the voltage
        float voltage;
        esp_err_t ret = adc_read_battery_voltage(&voltage);
        if (ret == ESP_OK) {
            ESP_LOGD("main", "Battery Counts: %.2f", voltage);
        } else {
            ESP_LOGE("main", "Failed to read battery voltage");
        }

//        // wait for the task to finish writing, but wait at max 50mS
//        for(int i= 0; i < 10; i++){
//            if(gap_get_write_in_progress() == false)
//                break;
//            else
//                vTaskDelay(pdMS_TO_TICKS(10));
//        }

        // write data to the car
        write_to_target_characteristic(data, 8);

        write_to_target_misc_characteristic(game_car_rgb, 3);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    ESP_LOGE("game_logic", "returning from game loop!");
}

// Test mode functions
void game_test_mode_button_pressed(int button_index) {
    // Only work in test mode (not connected)
    if (gap_ble_connected()) {
        return;
    }
    test_mode_button_pressed = true;
    test_mode_active_button = button_index;
    led_test_mode_button_pressed(button_index);
}

void game_test_mode_button_released(void) {
    // Only work in test mode (not connected)
    if (gap_ble_connected()) {
        return;
    }
    test_mode_button_pressed = false;
    test_mode_active_button = -1;
    led_test_mode_button_released();
}

bool game_test_mode_is_button_pressed(void) {
    return test_mode_button_pressed;
}

void game_test_mode_joystick_check(int adc_x, int adc_y) {
    // Only work in test mode (not connected)
    if (gap_ble_connected()) {
        return;
    }
    
    // Only check joystick if no button is currently pressed
    if (test_mode_button_pressed) {
        return; // Don't override button LED
    }
    
    // Check if joystick is at 50% or more on either axis
    int center_x = ADC_CENTER;
    int center_y = ADC_CENTER;
    int threshold = (ADC_MAX - ADC_MIN) / 4; // 25% of full range = 50% from center
    
    bool x_active = (abs(adc_x - center_x) > threshold);
    bool y_active = (abs(adc_y - center_y) > threshold);
    
    if (x_active && y_active) {
        // Both axes active - show X color (prioritize X)
        led_test_mode_joystick_x_active();
    } else if (x_active) {
        led_test_mode_joystick_x_active();
    } else if (y_active) {
        led_test_mode_joystick_y_active();
    } else {
        led_test_mode_joystick_inactive();
    }
}