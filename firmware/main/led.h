// led.h

#ifndef LED_H
#define LED_H

#include <stdbool.h>

#define NUM_LEDS 3

#define LED_1_GPIO 27 // connection
#define LED_2_GPIO 26 // low battery
#define LED_3_GPIO 25 // misc

void led_init(void);
void set_led(int led_index, bool state);

#endif // LED_H
