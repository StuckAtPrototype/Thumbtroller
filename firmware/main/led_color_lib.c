#include "led_color_lib.h"
#include <math.h>

static uint16_t hue_increment = 10;
static uint16_t current_hue = 0;

// Helper function to convert hue to RGB
static void hue_to_rgb(float h, float *r, float *g, float *b) {
    float x = 1 - fabsf(fmodf(h * 6, 2) - 1);

    if (h < 1.0f/6.0f)      { *r = 1; *g = x; *b = 0; }
    else if (h < 2.0f/6.0f) { *r = x; *g = 1; *b = 0; }
    else if (h < 3.0f/6.0f) { *r = 0; *g = 1; *b = x; }
    else if (h < 4.0f/6.0f) { *r = 0; *g = x; *b = 1; }
    else if (h < 5.0f/6.0f) { *r = x; *g = 0; *b = 1; }
    else                    { *r = 1; *g = 0; *b = x; }
}

uint32_t get_color_from_hue(uint16_t hue) {
    float h = hue / 65536.0f;
    float r, g, b;

    hue_to_rgb(h, &r, &g, &b);

    // Apply brightness limit
    r *= MAX_BRIGHTNESS * 255;
    g *= MAX_BRIGHTNESS * 255;
    b *= MAX_BRIGHTNESS * 255;

    // Convert to GRB format
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

uint32_t get_next_color_full_spectrum(void) {
    uint32_t color = get_color_from_hue(current_hue);

    // Increment hue for next call
    current_hue += hue_increment;
    if (current_hue >= 65536) current_hue = 0;

    return color;
}

void set_hue_increment(uint16_t increment) {
    hue_increment = increment;
}


uint32_t get_color_between_blue_red(float value) {
    float ratio;
    float r, g, b;

    // Ensure value is within the valid range
    if (value < COLOR_BLUE_HUE) value = COLOR_BLUE_HUE;
    if (value > COLOR_RED_HUE) value = COLOR_RED_HUE;

    // Calculate the ratio (0.0 for BLUE, 1.0 for RED)
    ratio = (value - COLOR_BLUE_HUE) / (COLOR_RED_HUE - COLOR_BLUE_HUE);

    // Interpolate between BLUE (0, 0, 1) and RED (1, 0, 0)
    r = ratio;
    g = 0.0f;
    b = 1.0f - ratio;

    // Apply brightness limit
    r *= MAX_BRIGHTNESS * 255;
    g *= MAX_BRIGHTNESS * 255;
    b *= MAX_BRIGHTNESS * 255;

    // Convert to GRB format
    return ((uint32_t)(g + 0.5f) << 16) | ((uint32_t)(r + 0.5f) << 8) | (uint32_t)(b + 0.5f);
}

static uint32_t pulse_time_ms = 0;
#define PULSE_MS 100  // 4 seconds

// Generic function for pulsing color effect
uint32_t get_pulsing_color(uint8_t red, uint8_t green, uint8_t blue) {
    // Calculate the phase of the pulse (0 to 2π)
    float phase = (pulse_time_ms / (float)PULSE_MS) * 2 * M_PI;

    // Use a sine wave to create a smooth pulse
    float brightness = (sinf(phase) + 1) / 4;

    // Apply the brightness to the specified color
    float r = brightness * red;
    float g = brightness * green;
    float b = brightness * blue;

    // Increment the time (function is called every 10ms)
    pulse_time_ms += 1;
    if (pulse_time_ms >= PULSE_MS) {
        pulse_time_ms = 0;  // Reset after PULSE_MS milliseconds
    }

    // Convert to GRB format
    return ((uint32_t)(g + 0.5f) << 16) | ((uint32_t)(r + 0.5f) << 8) | (uint32_t)(b + 0.5f);
}