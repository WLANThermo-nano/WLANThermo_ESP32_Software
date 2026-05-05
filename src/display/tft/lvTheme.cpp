/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "src/themes/lv_theme_private.h"
#include "lvTheme.h"

/*********************
 *      DEFINES
 *********************/

#define COLOR_SCR         (IS_LIGHT ? lv_color_hex(0xeaeff3) : lv_color_hex(0x181d23))
#define COLOR_SCR_TEXT    (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0xe7e9ec))

#define COLOR_BTN         (IS_LIGHT ? lv_color_hex(0xffffff) : lv_color_hex(0x0aa5c4))
#define COLOR_BTN_PR      (IS_LIGHT ? lv_color_hex(0xe0f4f8) : lv_color_hex(0x088eb0))
#define COLOR_BTN_CHK     (theme.color_primary)
#define COLOR_BTN_CHK_PR  (lv_color_darken(theme.color_primary, LV_OPA_30))
#define COLOR_BTN_DIS     (IS_LIGHT ? lv_color_hex(0xcccccc) : lv_color_hex(0x888888))

#define COLOR_BTN_BORDER  (theme.color_primary)
#define COLOR_BTN_BORDER_INA (IS_LIGHT ? lv_color_hex(0x888888) : lv_color_hex(0x404040))

#define COLOR_BG          (IS_LIGHT ? lv_color_hex(0xffffff) : lv_color_hex(0x29313a))
#define COLOR_BG_PR       (IS_LIGHT ? lv_color_hex(0xeeeeee) : lv_color_hex(0x494f57))
#define COLOR_BG_CHK      (theme.color_primary)
#define COLOR_BG_DIS      (COLOR_BG)
#define COLOR_BG_BORDER   (IS_LIGHT ? lv_color_hex(0xd6dde3) : lv_color_hex(0x808a97))
#define COLOR_BG_BORDER_CHK (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0x5f656e))
#define COLOR_BG_TEXT     (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0xffffff))
#define COLOR_BG_TEXT_CHK (lv_color_hex(0xffffff))
#define COLOR_BG_TEXT_DIS (IS_LIGHT ? lv_color_hex(0xaaaaaa) : lv_color_hex(0x999999))

#define COLOR_BG_SEC      (IS_LIGHT ? lv_color_hex(0xd4d7d9) : lv_color_hex(0x45494d))
#define COLOR_BG_SEC_BORDER (IS_LIGHT ? lv_color_hex(0xdfe7ed) : lv_color_hex(0x404040))
#define COLOR_BG_SEC_TEXT (IS_LIGHT ? lv_color_hex(0x31404f) : lv_color_hex(0xa5a8ad))

#define BORDER_WIDTH      LV_DPX(2)
#define IS_LIGHT          (theme.flags & LVTHEME_FLAG_LIGHT)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_style_t scr;

    lv_style_t bg;
    lv_style_t bg_pr;
    lv_style_t bg_chk;
    lv_style_t bg_dis;

    lv_style_t btn;
    lv_style_t btn_pr;
    lv_style_t btn_chk;
    lv_style_t btn_chk_pr;
    lv_style_t btn_dis;

    lv_style_t bar_bg;
    lv_style_t bar_indic;
    lv_style_t bar_indic_dis;

    lv_style_t slider_knob;

    lv_style_t sw_knob;

    lv_style_t roller_bg;
    lv_style_t roller_sel;

    lv_style_t spinbox_cursor;

    lv_style_t tab_bg;
    lv_style_t tab_btn;
    lv_style_t tab_btn_chk;
    lv_style_t tab_indic;

    lv_style_t mbox_bg;
} theme_styles_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;
static theme_styles_t *styles;
static bool inited;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void theme_apply(lv_theme_t *th, lv_obj_t *obj);

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void styles_init(void)
{
    /* Screen */
    lv_style_init(&styles->scr);
    lv_style_set_bg_opa(&styles->scr, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->scr, COLOR_SCR);
    lv_style_set_text_color(&styles->scr, COLOR_SCR_TEXT);
    lv_style_set_text_font(&styles->scr, theme.font_normal);

    /* Background / Container */
    lv_style_init(&styles->bg);
    lv_style_set_radius(&styles->bg, LV_DPX(8));
    lv_style_set_bg_opa(&styles->bg, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bg, COLOR_BG);
    lv_style_set_border_color(&styles->bg, COLOR_BG_BORDER);
    lv_style_set_border_width(&styles->bg, BORDER_WIDTH);
    lv_style_set_border_post(&styles->bg, true);
    lv_style_set_text_color(&styles->bg, COLOR_BG_TEXT);
    lv_style_set_text_font(&styles->bg, theme.font_normal);
    lv_style_set_line_color(&styles->bg, COLOR_BG_TEXT);
    lv_style_set_line_width(&styles->bg, 1);
    lv_style_set_pad_left(&styles->bg, LV_DPX(15) + BORDER_WIDTH);
    lv_style_set_pad_right(&styles->bg, LV_DPX(15) + BORDER_WIDTH);
    lv_style_set_pad_top(&styles->bg, LV_DPX(15) + BORDER_WIDTH);
    lv_style_set_pad_bottom(&styles->bg, LV_DPX(15) + BORDER_WIDTH);
    lv_style_set_pad_column(&styles->bg, LV_DPX(15));
    lv_style_set_pad_row(&styles->bg, LV_DPX(15));

    lv_style_init(&styles->bg_pr);
    lv_style_set_bg_color(&styles->bg_pr, COLOR_BG_PR);

    lv_style_init(&styles->bg_chk);
    lv_style_set_bg_color(&styles->bg_chk, COLOR_BG_CHK);
    lv_style_set_border_width(&styles->bg_chk, 0);
    lv_style_set_border_color(&styles->bg_chk, COLOR_BG_BORDER_CHK);
    lv_style_set_text_color(&styles->bg_chk, COLOR_BG_TEXT_CHK);

    lv_style_init(&styles->bg_dis);
    lv_style_set_bg_color(&styles->bg_dis, COLOR_BG_DIS);
    lv_style_set_text_color(&styles->bg_dis, COLOR_BG_TEXT_DIS);

    /* Button */
    lv_style_init(&styles->btn);
    lv_style_set_radius(&styles->btn, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&styles->btn, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->btn, COLOR_BTN);
    lv_style_set_border_color(&styles->btn, COLOR_BTN_BORDER);
    lv_style_set_border_width(&styles->btn, BORDER_WIDTH);
    lv_style_set_text_color(&styles->btn, IS_LIGHT ? lv_color_hex(0x31404f) : lv_color_hex(0xffffff));
    lv_style_set_pad_left(&styles->btn, LV_DPX(20));
    lv_style_set_pad_right(&styles->btn, LV_DPX(20));
    lv_style_set_pad_top(&styles->btn, LV_DPX(10));
    lv_style_set_pad_bottom(&styles->btn, LV_DPX(10));
    lv_style_set_pad_column(&styles->btn, LV_DPX(10));
    lv_style_set_pad_row(&styles->btn, LV_DPX(10));

    lv_style_init(&styles->btn_pr);
    lv_style_set_bg_color(&styles->btn_pr, COLOR_BTN_PR);

    lv_style_init(&styles->btn_chk);
    lv_style_set_bg_color(&styles->btn_chk, COLOR_BTN_CHK);
    lv_style_set_border_opa(&styles->btn_chk, LV_OPA_TRANSP);
    lv_style_set_text_color(&styles->btn_chk, lv_color_hex(0xffffff));

    lv_style_init(&styles->btn_chk_pr);
    lv_style_set_bg_color(&styles->btn_chk_pr, COLOR_BTN_CHK_PR);

    lv_style_init(&styles->btn_dis);
    lv_style_set_bg_color(&styles->btn_dis, COLOR_BTN_DIS);
    lv_style_set_border_color(&styles->btn_dis, COLOR_BTN_BORDER_INA);
    lv_style_set_text_color(&styles->btn_dis, lv_color_hex(0x888888));

    /* Bar / Progress bar */
    lv_style_init(&styles->bar_bg);
    lv_style_set_radius(&styles->bar_bg, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&styles->bar_bg, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bar_bg, COLOR_BG_SEC);

    lv_style_init(&styles->bar_indic);
    lv_style_set_bg_opa(&styles->bar_indic, LV_OPA_COVER);
    lv_style_set_radius(&styles->bar_indic, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&styles->bar_indic, theme.color_primary);

    lv_style_init(&styles->bar_indic_dis);
    lv_style_set_bg_color(&styles->bar_indic_dis, lv_color_hex(0x888888));

    /* Slider knob */
    lv_style_init(&styles->slider_knob);
    lv_style_set_bg_opa(&styles->slider_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->slider_knob, IS_LIGHT ? theme.color_primary : lv_color_white());
    lv_style_set_radius(&styles->slider_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_left(&styles->slider_knob, LV_DPX(7));
    lv_style_set_pad_right(&styles->slider_knob, LV_DPX(7));
    lv_style_set_pad_top(&styles->slider_knob, LV_DPX(7));
    lv_style_set_pad_bottom(&styles->slider_knob, LV_DPX(7));

    /* Switch knob */
    lv_style_init(&styles->sw_knob);
    lv_style_set_bg_opa(&styles->sw_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->sw_knob, lv_color_white());
    lv_style_set_radius(&styles->sw_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_top(&styles->sw_knob, -LV_DPX(4));
    lv_style_set_pad_bottom(&styles->sw_knob, -LV_DPX(4));
    lv_style_set_pad_left(&styles->sw_knob, -LV_DPX(4));
    lv_style_set_pad_right(&styles->sw_knob, -LV_DPX(4));

    /* Roller */
    lv_style_init(&styles->roller_bg);
    lv_style_set_text_line_space(&styles->roller_bg, LV_DPX(25));

    lv_style_init(&styles->roller_sel);
    lv_style_set_bg_opa(&styles->roller_sel, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->roller_sel, theme.color_primary);
    lv_style_set_text_color(&styles->roller_sel, lv_color_white());

    /* Spinbox cursor */
    lv_style_init(&styles->spinbox_cursor);
    lv_style_set_bg_opa(&styles->spinbox_cursor, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->spinbox_cursor, theme.color_primary);
    lv_style_set_text_color(&styles->spinbox_cursor, lv_color_white());

    /* Tabview */
    lv_style_init(&styles->tab_bg);
    lv_style_set_bg_opa(&styles->tab_bg, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tab_bg, COLOR_BG);
    lv_style_set_text_color(&styles->tab_bg, COLOR_SCR_TEXT);
    lv_style_set_pad_top(&styles->tab_bg, LV_DPX(7));
    lv_style_set_pad_left(&styles->tab_bg, LV_DPX(7));
    lv_style_set_pad_right(&styles->tab_bg, LV_DPX(7));

    lv_style_init(&styles->tab_btn);
    lv_style_set_pad_top(&styles->tab_btn, LV_DPX(10));
    lv_style_set_pad_bottom(&styles->tab_btn, LV_DPX(10));

    lv_style_init(&styles->tab_btn_chk);
    lv_style_set_text_color(&styles->tab_btn_chk, COLOR_SCR_TEXT);

    lv_style_init(&styles->tab_indic);
    lv_style_set_bg_opa(&styles->tab_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tab_indic, theme.color_primary);
    lv_style_set_height(&styles->tab_indic, LV_DPX(5));

    /* Message box */
    lv_style_init(&styles->mbox_bg);
    lv_style_set_shadow_width(&styles->mbox_bg, LV_DPX(30));
    lv_style_set_shadow_color(&styles->mbox_bg, IS_LIGHT ? lv_color_hex(0xc0c0c0) : lv_color_hex(0x999999));
}

static void theme_apply(lv_theme_t *th, lv_obj_t *obj)
{
    LV_UNUSED(th);

    const lv_obj_class_t *cls = lv_obj_get_class(obj);

    if (cls == &lv_obj_class) {
        if (lv_obj_get_parent(obj) == NULL) {
            /* Screen */
            lv_obj_add_style(obj, &styles->scr, 0);
        } else {
            /* Plain object / container */
            lv_obj_add_style(obj, &styles->bg, 0);
            lv_obj_add_style(obj, &styles->bg_pr, LV_STATE_PRESSED);
            lv_obj_add_style(obj, &styles->bg_chk, LV_STATE_CHECKED);
            lv_obj_add_style(obj, &styles->bg_dis, LV_STATE_DISABLED);
        }
        return;
    }

#if LV_USE_BTN
    if (cls == &lv_btn_class) {
        lv_obj_add_style(obj, &styles->btn, 0);
        lv_obj_add_style(obj, &styles->btn_pr, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->btn_chk, LV_STATE_CHECKED);
        lv_obj_add_style(obj, &styles->btn_chk_pr, LV_STATE_CHECKED | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->btn_dis, LV_STATE_DISABLED);
        return;
    }
#endif

#if LV_USE_BAR
    if (cls == &lv_bar_class) {
        lv_obj_add_style(obj, &styles->bar_bg, 0);
        lv_obj_add_style(obj, &styles->bar_indic, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->bar_indic_dis, LV_PART_INDICATOR | LV_STATE_DISABLED);
        return;
    }
#endif

#if LV_USE_SLIDER
    if (cls == &lv_slider_class) {
        lv_obj_add_style(obj, &styles->bar_bg, 0);
        lv_obj_add_style(obj, &styles->bar_indic, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->slider_knob, LV_PART_KNOB);
        return;
    }
#endif

#if LV_USE_SWITCH
    if (cls == &lv_switch_class) {
        lv_obj_add_style(obj, &styles->bar_bg, 0);
        lv_obj_add_style(obj, &styles->bar_indic, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->bar_indic, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &styles->sw_knob, LV_PART_KNOB);
        return;
    }
#endif

#if LV_USE_ROLLER
    if (cls == &lv_roller_class) {
        lv_obj_add_style(obj, &styles->bg, 0);
        lv_obj_add_style(obj, &styles->roller_bg, 0);
        lv_obj_add_style(obj, &styles->roller_sel, LV_PART_SELECTED);
        return;
    }
#endif

#if LV_USE_SPINBOX
    if (cls == &lv_spinbox_class) {
        lv_obj_add_style(obj, &styles->bg, 0);
        lv_obj_add_style(obj, &styles->spinbox_cursor, LV_PART_CURSOR);
        return;
    }
#endif

#if LV_USE_TABVIEW
    if (cls == &lv_tabview_class) {
        lv_obj_add_style(obj, &styles->scr, 0);
        lv_obj_add_style(obj, &styles->tab_bg, LV_PART_ITEMS);
        lv_obj_add_style(obj, &styles->tab_btn, LV_PART_ITEMS);
        lv_obj_add_style(obj, &styles->tab_btn_chk, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &styles->tab_indic, LV_PART_INDICATOR);
        return;
    }
#endif

#if LV_USE_MSGBOX
    if (cls == &lv_msgbox_class) {
        lv_obj_add_style(obj, &styles->bg, 0);
        lv_obj_add_style(obj, &styles->mbox_bg, 0);
        return;
    }
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_theme_t *lvTheme_Init(lv_display_t *disp, lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                         const lv_font_t *font_small, const lv_font_t *font_normal, const lv_font_t *font_subtitle,
                         const lv_font_t *font_title)
{
    if (!inited) {
        styles = (theme_styles_t *)lv_malloc(sizeof(theme_styles_t));
    }

    theme.color_primary   = color_primary;
    theme.color_secondary = color_secondary;
    theme.font_small      = font_small;
    theme.font_normal     = font_normal;
    theme.font_large      = font_subtitle;
    theme.flags           = flags;

    styles_init();

    lv_theme_set_apply_cb(&theme, theme_apply);
    lv_display_set_theme(disp, &theme);

    inited = true;

    return &theme;
}
