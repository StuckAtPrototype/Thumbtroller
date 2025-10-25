#ifndef BUTTON_ISR_H
#define BUTTON_ISR_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Define debounce time in microseconds since esp_timer_get_time() returns microseconds
#define DEBOUNCE_TIME_US (50000)  // 50ms in microseconds

typedef struct {
    gpio_num_t pin;
    bool pressed;
    int64_t last_edge_time;
} ButtonState;

void button_isr_init(gpio_num_t *pins, uint8_t count, QueueHandle_t event_queue);
QueueHandle_t button_get_event_queue(void);
extern void button_isr_debounce_task(void *arg);

#endif // BUTTON_ISR_H


