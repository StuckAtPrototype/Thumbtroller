#ifndef GPIO_INTERRUPT_H
#define GPIO_INTERRUPT_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Define the interrupt pin
#define INTERRUPT_PIN_A 5
#define INTERRUPT_PIN_B 10

// Function prototypes
void configure_gpio_interrupt(void);
void start_gpio_interrupt_task(void);

// Declare the queue handle as extern so it can be accessed from main.c
extern QueueHandle_t gpio_evt_queue;

#endif // GPIO_INTERRUPT_H
