// SPDX-License-Identifier: MIT
// Display interface.

extern "C" {
#include "display.h"

#include "resources.h"
}

#define LGFX_WT32_SC01
#include <LGFX_AUTODETECT.hpp>

// LCD handle
static LGFX lcd;

// Currently displayed info
static struct info curr_info = {
    .wifi = false,
    .ntp = false,
    .hours = 255,
    .minutes = 255,
    .seconds = 255,
    .temperature = 0,
    .humidity = 0,
    .pressure = 0,
};

/**
 * Draw masked image.
 * @param font pointer to the image instance to use
 * @param x,y coordinates of the left top corner
 * @param color output color
 */
static void draw_image(const struct image* img, size_t x, size_t y,
                       uint32_t color)
{
    for (size_t dy = 0; dy < img->height; ++dy) {
        const size_t disp_y = dy + y;
        for (size_t dx = 0; dx < img->width; ++dx) {
            const size_t disp_x = dx + x;
            const uint32_t pixel = color * image_bit(img, dx, dy);
            lcd.writePixel(disp_x, disp_y, pixel);
        }
    }
}

/**
 * Draw masked image from the font.
 * @param font pointer to the font instance to use
 * @param index index of the font symbol
 * @param x,y coordinates of the left top corner
 * @param color output color
 */
static void draw_font(const struct font* font, size_t index, size_t x, size_t y,
                      uint32_t color)
{
    for (size_t dy = 0; dy < font->height; ++dy) {
        const size_t disp_y = dy + y;
        for (size_t dx = 0; dx < font->width; ++dx) {
            const size_t disp_x = dx + x;
            const uint32_t pixel = color * font_bit(font, index, dx, dy);
            lcd.writePixel(disp_x, disp_y, pixel);
        }
    }
}

/**
 * Draw number.
 * @param font pointer to the font instance to use
 * @param x,y coordinates of the left top corner
 * @param color output color
 * @param value number to draw
 * @param min_digits minimal number of digits to draw
 */
static void draw_number(const struct font* font, size_t x, size_t y,
                        uint32_t color, size_t value, size_t min_digits)
{
    size_t bcd = 0;
    size_t digits = 0;
    while (value > 0) {
        bcd |= (value % 10) << (digits * 4);
        value /= 10;
        ++digits;
    }
    if (digits < min_digits) {
        digits = min_digits;
    }

    for (size_t i = 0; i < digits; ++i) {
        const size_t start_bit = (digits - i - 1) * 4;
        const uint8_t digit = (bcd >> start_bit) & 0xf;
        const size_t x_offset = x + (font->width + font->spacing) * i;
        draw_font(font, digit, x_offset, y, color);
    }
}

/**
 * Fill rectangle with specified color.
 * @param x,y coordinates of the left top corner
 * @param width,height size of the rectangle
 * @param color output color
 */
static void fill(size_t x, size_t y, size_t width, size_t height,
                 uint32_t color)
{
    const size_t max_x = x + width;
    const size_t max_y = y + height;
    for (; y < max_y; ++y) {
        for (size_t dx = x; dx < max_x; ++dx) {
            lcd.writePixel(dx, y, color);
        }
    }
}

extern "C" void display_init(void)
{
    lcd.init();
    lcd.setRotation(1);
    lcd.setColorDepth(lgfx::rgb888_3Byte);
    lcd.setBrightness(16);
}

extern "C" void display_redraw(const struct info* info)
{
    const bool first_run = curr_info.hours >= 24;
    const uint32_t main_color = lcd.color888(0xff, 0, 0);

    lcd.startWrite();

    // wifi/ntp status
    if (first_run || curr_info.wifi != info->wifi ||
        curr_info.ntp != info->ntp) {
        const struct image* img = NULL;
        if (!info->wifi) {
            img = get_image(image_wifi);
        } else if (!info->ntp) {
            img = get_image(image_ntp);
        }
        if (img) {
            draw_image(img, DISPLAY_WIDTH / 2 - 10, 0,
                       lcd.color888(0xff, 0xff, 0));
        } else {
            fill(DISPLAY_WIDTH / 2 - 10, 0, 20, 20, lcd.color888(0, 0, 0));
        }
    }

    // static elements
    if (first_run) {
        const uint32_t clr = lcd.color888(0xa0, 0x00, 0x00);
        draw_image(get_image(image_celsius), 50, 250, clr);
        draw_image(get_image(image_percent), 155, 250, clr);
        draw_image(get_image(image_mm), 260, 250, clr);

        fill(DISPLAY_WIDTH / 2 - 10, 50, 20, 20, main_color);
        fill(DISPLAY_WIDTH / 2 - 10, 100, 20, 20, main_color);
    }

    // current time
    if (curr_info.hours != info->hours) {
        draw_number(get_font(font100), 10, 10, main_color, info->hours, 2);
    }
    if (curr_info.minutes != info->minutes) {
        draw_number(get_font(font100), 270, 10, main_color, info->minutes, 2);
    }
    draw_number(get_font(font60), 350, 190, main_color, info->seconds, 2);

    // temperature/humidity/pressure
    draw_number(get_font(font28), 40, 200, main_color, info->temperature, 2);
    draw_number(get_font(font28), 140, 200, main_color, info->humidity, 2);
    const float pressure_mm = info->pressure / 1.3332;
    draw_number(get_font(font28), 230, 200, main_color, pressure_mm, 3);

    lcd.endWrite();

    curr_info = *info;
}
