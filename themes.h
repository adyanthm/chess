#ifndef THEMES_H
#define THEMES_H

#include <raylib.h>

typedef struct {
    const char *name;
    Color bg;
    Color light, dark;
    Color selection_light, selection_dark;
    Color indicator_light, indicator_dark;
} Theme;

static const Theme THEME_GREEN = {
    .name            = "green",
    .bg              = { 48,  46,  43, 255},
    .light           = {235, 236, 208, 255},
    .dark            = {115, 149,  82, 255},
    .selection_light = {245, 246, 130, 255},
    .selection_dark  = {185, 202,  67, 255},
    .indicator_light = {202, 203, 179, 255},
    .indicator_dark  = { 99, 128,  70, 255},
};

static const Theme THEME_LICHESS = {
    .name            = "lichess",
    .bg              = { 40,  38,  36, 255},
    .light           = {240, 217, 181, 255},
    .dark            = {181, 136,  99, 255},
    .selection_light = {205, 210, 106, 255},
    .selection_dark  = {170, 162,  58, 255},
    .indicator_light = {214, 189, 152, 255},
    .indicator_dark  = {157, 115,  80, 255},
};

static const Theme THEME_MODERN = {
    .name            = "modern",
    .bg              = { 38,  42,  48, 255},
    .light           = {222, 227, 230, 255},
    .dark            = {140, 162, 173, 255},
    .selection_light = {246, 232, 130, 255},
    .selection_dark  = {214, 205, 100, 255},
    .indicator_light = {192, 200, 206, 255},
    .indicator_dark  = {112, 134, 146, 255},
};

static const Theme *const THEMES[] = { &THEME_GREEN, &THEME_LICHESS, &THEME_MODERN };
#define THEME_COUNT ((int)(sizeof THEMES / sizeof THEMES[0]))

#endif
