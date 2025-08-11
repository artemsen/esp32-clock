// SPDX-License-Identifier: MIT
// Images and fonts resources.

#include "resources.h"

#include "img/celsius.xbm"
#include "img/font100.xbm"
#include "img/font28.xbm"
#include "img/font60.xbm"
#include "img/mm.xbm"
#include "img/ntp.xbm"
#include "img/percent.xbm"
#include "img/wifi.xbm"

#define XBM_STRIDE(w) (((w + 7) / 8) * 8)

#define FONT_SYMBOLS    10
#define FONT_SPACING(w) (w / FONT_SYMBOLS / 20)

static const struct font fonts[] = {
    [font28] = {
        .width = font28_width / FONT_SYMBOLS,
        .height = font28_height,
        .spacing = FONT_SPACING(font28_width),
        .stride = XBM_STRIDE(font28_width),
        .mask = font28_bits,
    },
    [font60] = {
        .width = font60_width / FONT_SYMBOLS,
        .height = font60_height,
        .spacing = FONT_SPACING(font60_height),
        .stride = XBM_STRIDE(font60_width),
        .mask = font60_bits,
    },
    [font100] = {
        .width = font100_width / FONT_SYMBOLS,
        .height = font100_height,
        .spacing = FONT_SPACING(font100_height),
        .stride = XBM_STRIDE(font100_width),
        .mask = font100_bits,
    },
};

static const struct image images[] = {
    [image_wifi] = {
        .width = wifi_width,
        .height = wifi_height,
        .stride = XBM_STRIDE(wifi_width),
        .mask = wifi_bits,
    },
    [image_ntp] = {
        .width = ntp_width,
        .height = ntp_height,
        .stride = XBM_STRIDE(wifi_width),
        .mask = ntp_bits,
    },
    [image_celsius] = {
        .width = celsius_width,
        .height = celsius_height,
        .stride = XBM_STRIDE(celsius_width),
        .mask = celsius_bits,
    },
    [image_percent] = {
        .width = percent_width,
        .height = percent_height,
        .stride = XBM_STRIDE(percent_width),
        .mask = percent_bits,
    },
    [image_mm] = {
        .width = mm_width,
        .height = mm_height,
        .stride = XBM_STRIDE(mm_width),
        .mask = mm_bits,
    },
};

const struct image* get_image(enum image_type type)
{
    return &images[type];
}

const struct font* get_font(enum font_size size)
{
    return &fonts[size];
}
