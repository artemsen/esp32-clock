// SPDX-License-Identifier: MIT
// Images and fonts resources.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Supported images
enum image_type {
    image_wifi,
    image_ntp,
    image_celsius,
    image_percent,
    image_mm,
};

// Supported fonts (digits 0-9 only)
enum font_size {
    font28,
    font60,
    font100,
};

// Masked image description
struct image {
    size_t width;
    size_t height;
    size_t stride;
    const uint8_t* mask;
};

// Masked font description
struct font {
    size_t width;
    size_t height;
    size_t spacing;
    size_t stride;
    const uint8_t* mask;
};

/**
 * Get masked image instance.
 * @param type image type
 * @return image instance
 */
const struct image* get_image(enum image_type type);

/**
 * Get masked font instance.
 * @param size font size
 * @return font instance
 */
const struct font* get_font(enum font_size size);

/**
 * Get image bit value.
 * @param img pointer to the image instance
 * @param x,y coordinates of the left top corner
 * @return masked image bit value
 */
static inline bool image_bit(const struct image* img, size_t x, size_t y)
{
    const size_t bit_index = y * img->stride + x;
    const size_t bit_nbyte = bit_index / 8;
    return (img->mask[bit_nbyte] >> (bit_index % 8)) & 1;
}

/**
 * Get font bit value.
 * @param font pointer to the font instance to use
 * @param index index of the font symbol
 * @param x,y coordinates of the left top corner
 * @return masked font bit value
 */
static inline bool font_bit(const struct font* font, size_t index, size_t x,
                            size_t y)
{
    const size_t bit_index = y * font->stride + index * font->width + x;
    const size_t bit_nbyte = bit_index / 8;
    return (font->mask[bit_nbyte] >> (bit_index % 8)) & 1;
}
