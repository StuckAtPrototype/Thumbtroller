#include "ws2812_control.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#define LED_RMT_TX_GPIO         25
#define BITS_PER_LED_CMD        24
#define LED_BUFFER_ITEMS        (NUM_LEDS * BITS_PER_LED_CMD)

// Assuming 10MHz resolution
#define T0H     3  // 0 bit high time
#define T1H     6  // 1 bit high time
#define T0L     8  // 0 bit low time
#define T1L     5  // 1 bit low time

static const char *TAG = "NeoPixel WS2812 Driver";

static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t led_encoder = NULL;
static uint8_t led_data_buffer[NUM_LEDS * 3]; // 3 bytes per LED (RGB)

static size_t ws2812_rmt_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_encoder_handle_t bytes_encoder = (rmt_encoder_handle_t)encoder;
    rmt_encode_state_t session_state = 0;
    size_t encoded_symbols = bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
    *ret_state = session_state;
    return encoded_symbols;
}

esp_err_t ws2812_control_init(void)
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = LED_RMT_TX_GPIO,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10 * 1000 * 1000, // 10MHz resolution
            .mem_block_symbols = 64,
            .trans_queue_depth = 4,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&tx_chan_config, &led_chan), TAG, "Failed to create RMT TX channel");

    ESP_LOGI(TAG, "Install led strip encoder");
    rmt_bytes_encoder_config_t bytes_encoder_config = {
            .bit0 = {
                    .level0 = 1,
                    .duration0 = T0H,
                    .level1 = 0,
                    .duration1 = T0L,
            },
            .bit1 = {
                    .level0 = 1,
                    .duration0 = T1H,
                    .level1 = 0,
                    .duration1 = T1L,
            },
            .flags.msb_first = 1,
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder), TAG, "Failed to create bytes encoder");

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_RETURN_ON_ERROR(rmt_enable(led_chan), TAG, "Failed to enable RMT channel");

    return ESP_OK;
}

esp_err_t ws2812_write_leds(struct led_state new_state)
{
    for (uint32_t led = 0; led < NUM_LEDS; led++) {
        uint32_t bits_to_send = new_state.leds[led];
        led_data_buffer[led * 3 + 0] = (bits_to_send >> 16) & 0xFF; // Red
        led_data_buffer[led * 3 + 1] = (bits_to_send >> 8) & 0xFF;  // Green
        led_data_buffer[led * 3 + 2] = bits_to_send & 0xFF;         // Blue
    }

    rmt_transmit_config_t tx_config = {
            .loop_count = 0,
    };

    ESP_RETURN_ON_ERROR(rmt_transmit(led_chan, led_encoder, led_data_buffer, sizeof(led_data_buffer), &tx_config), TAG, "Failed to transmit RMT data");
    ESP_RETURN_ON_ERROR(rmt_tx_wait_all_done(led_chan, portMAX_DELAY), TAG, "Failed to wait for RMT transmission to finish");

    return ESP_OK;
}
