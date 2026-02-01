#ifndef LED_COLOR_LIB_H
#define LED_COLOR_LIB_H

#include <stdint.h>

// Define the maximum brightness (1.0 == 100%)
#define MAX_BRIGHTNESS 1.0f

// Define color codes
// these are temperature ranges
#define COLOR_BLUE_HUE 17.0f
#define COLOR_RED_HUE 27.0f

// Function to get a color based on a specific hue
uint32_t get_color_from_hue(uint16_t hue);

// Function to get the next color in the spectrum
uint32_t get_next_color_full_spectrum(void);

// Function to set the hue increment for the next_color function
void set_hue_increment(uint16_t increment);

// New function to get color between BLUE and RED
uint32_t get_color_between_blue_red(float value);

uint32_t get_pulsing_color(uint8_t red, uint8_t green, uint8_t blue);

#endif // LED_COLOR_LIB_H
