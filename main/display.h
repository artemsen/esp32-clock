// SPDX-License-Identifier: MIT
// Display interface.

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" { // LGFX is C++
#endif

/**
 * Initialize LCD display.
 */
void display_init(void);

/**
 * Set brightness and color.
 * @param brightness value to set (0-255)
 * @param r,g,b output color
 */
void display_setup(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b);

/**
 * Update time on display.
 */
void display_update(void);

#ifdef __cplusplus
} // extern "C"
#endif
