// SPDX-License-Identifier: MIT
// Display interface.

#include "display.h"

#include "img/digit0b.xbm"
#include "img/digit1b.xbm"
#include "img/digit2b.xbm"
#include "img/digit3b.xbm"
#include "img/digit4b.xbm"
#include "img/digit5b.xbm"
#include "img/digit6b.xbm"
#include "img/digit7b.xbm"
#include "img/digit8b.xbm"
#include "img/digit9b.xbm"
#include "img/digit0s.xbm"
#include "img/digit1s.xbm"
#include "img/digit2s.xbm"
#include "img/digit3s.xbm"
#include "img/digit4s.xbm"
#include "img/digit5s.xbm"
#include "img/digit6s.xbm"
#include "img/digit7s.xbm"
#include "img/digit8s.xbm"
#include "img/digit9s.xbm"
#include "img/delim_b.xbm"
#include "img/delim_s.xbm"

#define LGFX_WT32_SC01
#include <LGFX_AUTODETECT.hpp>

// Display size
static constexpr uint32_t display_width = 480;
static constexpr uint32_t display_height = 320;

// Display context: LCD state, current color and showed time
static LGFX lcd;
static uint32_t current_color;
static struct tm current_time;

// Digits description (something like a font)
struct digit {
    uint32_t width;
    uint32_t height;
    uint32_t space;          // space between symbols
    const uint8_t* mask[10]; // bits for each digit 0-9
};

// Big digits
static const struct digit big_digits = {
    80, 140, 4, {
        digit0b_bits, digit1b_bits, digit2b_bits, digit3b_bits, digit4b_bits,
        digit5b_bits, digit6b_bits, digit7b_bits, digit8b_bits, digit9b_bits
    }
};

// Small digits
static const struct digit small_digits = {
    40, 64, 2, {
        digit0s_bits, digit1s_bits, digit2s_bits, digit3s_bits, digit4s_bits,
        digit5s_bits, digit6s_bits, digit7s_bits, digit8s_bits, digit9s_bits
    }
};

/**
 * Fill bit mask with the current color.
 * @param x,y coordinates of the left top corner
 * @param width,height size of area
 * @param mask pointer to bit mask
 */
static void fill_bits(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t* mask)
{
    for (uint32_t dy = 0; dy < height; ++dy) {
        const uint32_t disp_y = y + dy;
        for (uint32_t dx = 0; dx < width; ++dx) {
            const uint32_t disp_x = x + dx;
            const size_t bit_index = dy * width + dx;
            const size_t bit_nbyte = bit_index / 8;
            const uint8_t bit_val = (mask[bit_nbyte] >> (bit_index % 8)) & 1;
            const uint32_t pixel = bit_val * current_color;
            lcd.writePixel(disp_x, disp_y, pixel);
        }
    }
}

/**
 * Draw two-digit number.
 * @param x,y coordinates of the left top corner
 * @param dig pointer to digits description
 * @param num two-digit number to draw (0-99)
 */
static void draw_number(uint32_t x, uint32_t y, const struct digit* dig, uint8_t num)
{
    const uint8_t first = num / 10;
    const uint8_t second = num - first * 10;
    fill_bits(x, y, dig->width, dig->height, dig->mask[first]);
    fill_bits(x + dig->width + dig->space, y, dig->width, dig->height, dig->mask[second]);
}

extern "C" void display_init(void)
{
    lcd.init();
    lcd.setRotation(1);
    lcd.setColorDepth(lgfx::rgb888_3Byte);
    memset(&current_time, 0xff, sizeof(current_time));
}

extern "C" void display_setup(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b)
{
    lcd.setBrightness(brightness);
    current_color = lcd.color888(r, g, b);
    memset(&current_time, 0xff, sizeof(current_time)); // force redraw
}

extern "C" void display_update(void)
{
    time_t now;
    struct tm previous;

    memcpy(&previous, &current_time, sizeof(current_time));
    time(&now);
    localtime_r(&now, &current_time);

    lcd.startWrite();

    // draw time
    if (current_time.tm_hour != previous.tm_hour) {
        draw_number(34, 40, &big_digits, current_time.tm_hour);
        fill_bits(200, 40, delim_b_width, delim_b_height, delim_b_bits);
    }
    if (current_time.tm_min != previous.tm_min) {
        draw_number(282, 40, &big_digits, current_time.tm_min);
    }
    draw_number(360, 200, &small_digits, current_time.tm_sec);

    lcd.endWrite();
}
