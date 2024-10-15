// led.c

#include "led.h"
#include "driver/gpio.h"
#include <stdio.h>

static bool led_state[NUM_LEDS] = {false};

static void configure_led(int gpio)
{
    gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << gpio),
            .pull_down_en = 0,
            .pull_up_en = 0
    };
    gpio_config(&io_conf);
}

void led_init(void)
{
    configure_led(LED_1_GPIO);
    configure_led(LED_2_GPIO);
    configure_led(LED_3_GPIO);
}

void set_led(int led_index, bool state)
{
    if (led_index < 0 || led_index >= NUM_LEDS) {
        printf("Invalid LED index\n");
        return;
    }

    int gpio_num;
    switch(led_index) {
        case 0: gpio_num = LED_1_GPIO; break;
        case 1: gpio_num = LED_2_GPIO; break;
        case 2: gpio_num = LED_3_GPIO; break;
        default:
            printf("Invalid LED index\n");
            return;
    }

    led_state[led_index] = state;
    gpio_set_level(gpio_num, state ? 1 : 0);
    printf("LED %d (GPIO %d) set to %s\n", led_index, gpio_num, state ? "ON" : "OFF");
}
