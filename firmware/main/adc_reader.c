#include "adc_reader.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "adc_reader";

#define ADC_UNIT ADC_UNIT_1
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_DEFAULT

static adc_oneshot_unit_handle_t adc1_handle;

esp_err_t adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create new ADC unit");
        return ret;
    }

    adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN,
            .bitwidth = ADC_BITWIDTH,
    };

    ret = adc_oneshot_config_channel(adc1_handle, THUMB_ADC_CHANNEL_0, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel 0");
        return ret;
    }

    ret = adc_oneshot_config_channel(adc1_handle, THUMB_ADC_CHANNEL_1, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel 1");
        return ret;
    }

    ret = adc_oneshot_config_channel(adc1_handle, THUMB_ADC_CHANNEL_2, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel 2");
        return ret;
    }

    ESP_LOGI(TAG, "ADC initialized successfully");
    return ESP_OK;
}

esp_err_t read_adc(adc_channel_t channel, int *adc_reading)
{
    if (channel != THUMB_ADC_CHANNEL_0 && channel != THUMB_ADC_CHANNEL_1 && channel != THUMB_ADC_CHANNEL_2) {
        ESP_LOGE(TAG, "Invalid ADC channel: %d", channel);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = adc_oneshot_read(adc1_handle, channel, adc_reading);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC read failed on channel %d", channel);
        return ret;
    }

    ESP_LOGD(TAG, "ADC Channel %d Raw: %d", channel, *adc_reading);
    return ESP_OK;
}

esp_err_t adc_read_battery_voltage(float *voltage)
{
   int raw;
   esp_err_t ret = read_adc(THUMB_ADC_CHANNEL_2, &raw);
   if (ret != ESP_OK) {
       return ret;
   }

   // Convert ADC raw reading to voltage applying your calibration formula if needed
   // Example placeholder formula from your earlier code (uncomment and adjust as needed):
   // *voltage = (1.4925f * raw - 511.83f) / 1000.0f;

   // For now, just cast raw ADC value to float
   *voltage = (float)raw;

//    ESP_LOGD(TAG, "Battery Voltage: %.3f V", *voltage);

    return ESP_OK;
}
