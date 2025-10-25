#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

// Define ADC channels using ESP-IDF channel declarations
// Using THUMB_ prefix to avoid conflicts with ESP-IDF naming convention
#define THUMB_ADC_CHANNEL_0 ADC_CHANNEL_0   // X-axis thumbstick (ADC1_CHANNEL_0)
#define THUMB_ADC_CHANNEL_1 ADC_CHANNEL_3   // Y-axis thumbstick (ADC1_CHANNEL_3)
#define THUMB_ADC_CHANNEL_2 ADC_CHANNEL_4   // Battery voltage (ADC1_CHANNEL_4)

// Function prototypes
esp_err_t adc_init(void);
esp_err_t read_adc(adc_channel_t channel, int *adc_reading);
esp_err_t adc_read_battery_voltage(float *voltage);

#endif // ADC_READER_H
