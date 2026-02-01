#include "button_isr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "game_logic.h"
#include "gap.h"

static const char *TAG = "BTN_ISR";
static ButtonState *buttons;
static uint8_t button_count;
static QueueHandle_t event_queue;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    ButtonState *btn = (ButtonState *)arg;
    int64_t now = esp_timer_get_time();

    if ((now - btn->last_edge_time) > DEBOUNCE_TIME_US) {
        // For active-high buttons: level 1 means pressed
        bool current_state = gpio_get_level(btn->pin) == 1;
        if (current_state != btn->pressed) {
            btn->pressed = current_state;
            btn->last_edge_time = now;
            xQueueSendFromISR(event_queue, &btn->pin, NULL);
        }
    }
}

void button_isr_init(gpio_num_t *pins, uint8_t count, QueueHandle_t queue) {
    buttons = (ButtonState *)malloc(sizeof(ButtonState) * count);
    button_count = count;
    event_queue = queue;

    gpio_install_isr_service(0);

    for (int i = 0; i < count; i++) {
        buttons[i].pin = pins[i];
        buttons[i].pressed = false;
        buttons[i].last_edge_time = 0;

        gpio_config_t io_conf = {
                .pin_bit_mask = (1ULL << pins[i]),
                .mode = GPIO_MODE_INPUT,
                // Disable internal pull-ups since external pull-downs are used
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_ENABLE,
                .intr_type = GPIO_INTR_ANYEDGE
        };
        gpio_config(&io_conf);

        gpio_isr_handler_add(pins[i], gpio_isr_handler, (void *)&buttons[i]);
    }
}

void button_isr_debounce_task(void *arg) {
    gpio_num_t pin;
    while (1) {
        if (xQueueReceive(event_queue, &pin, portMAX_DELAY)) {
            // Find which button triggered
            for (int i = 0; i < button_count; i++) {
                if (buttons[i].pin == pin) {
                    // Log the button state change
                    if (buttons[i].pressed) {
                        ESP_LOGI(TAG, "Button %d is PRESSED", i);

                        // Check if we're in test mode (not connected to car)
                        if (!gap_connected()) {
                            // Test mode: show different colors for each button
                            game_test_mode_button_pressed(i);
                        } else {
                            // Normal mode: handle buttons as before
                            if(i == 0){
                                // color change
                                game_change_car_color();
                            } else if(i == 1){
                                // mode change
                                game_toggle_control_mode();
                            } else if(i == 2){
                                // button 2 - 50% reverse (mode 1 only)
                                game_button_2_pressed();
                            } else if(i == 3){
                                // button 3 - full forward (mode 1 only)
                                game_button_3_pressed();
                            }
                        }

                    } else {
                        ESP_LOGI(TAG, "Button %d is RELEASED", i);
                        
                        // Check if we're in test mode (not connected to car)
                        if (!gap_connected()) {
                            // Test mode: turn off LED when button released
                            game_test_mode_button_released();
                        } else {
                            // Normal mode: handle button releases as before
                            if(i == 2){
                                // button 2 released
                                game_button_2_released();
                            } else if(i == 3){
                                // button 3 released
                                game_button_3_released();
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}