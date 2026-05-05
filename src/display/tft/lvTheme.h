#pragma once
/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    LVTHEME_FLAG_DARK = 0x01,
    LVTHEME_FLAG_LIGHT = 0x02,
    LVTHEME_FLAG_NO_TRANSITION = 0x10,
    LVTHEME_FLAG_NO_FOCUS = 0x20,
} lvTheme_FlagType;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_theme_t *lvTheme_Init(lv_display_t *disp, lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                         const lv_font_t *font_small, const lv_font_t *font_normal, const lv_font_t *font_subtitle,
                         const lv_font_t *font_title);
