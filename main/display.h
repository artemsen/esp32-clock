// SPDX-License-Identifier: MIT
// Display interface.

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Display size
#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320

/**
 * Info to display.
 */
struct info {
    bool wifi;
    bool ntp;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    float temperature;
    float humidity;
    float pressure;
};

/**
 * Initialize LCD display.
 */
void display_init(void);

/**
 * Redraw display.
 * @param info values to display
 */
void display_redraw(const struct info* info);
