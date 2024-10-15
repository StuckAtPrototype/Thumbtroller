#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/adc.h"

// ADC channel definitions
#define ADC_CHANNEL_0 ADC1_CHANNEL_0  // IO1
#define ADC_CHANNEL_1 ADC1_CHANNEL_3  // IO4
#define ADC_CHANNEL_2 ADC1_CHANNEL_2  // IO2 (Battery)

// Function prototypes
esp_err_t adc_init(void);
esp_err_t read_adc(adc1_channel_t channel, int *adc_reading);
esp_err_t read_battery_voltage(float *voltage);

#endif // ADC_READER_H
