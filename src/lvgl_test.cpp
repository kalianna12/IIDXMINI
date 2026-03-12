#include "lvgl_test.h"

static lv_obj_t * full_screen_obj = nullptr;

static void color_timer_cb(lv_timer_t * t)
{
    static uint8_t idx = 0;

    static const lv_color_t colors[] = {
        lv_palette_main(LV_PALETTE_RED),
        lv_palette_main(LV_PALETTE_GREEN),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_YELLOW),
        lv_palette_main(LV_PALETTE_ORANGE),
        lv_palette_main(LV_PALETTE_PURPLE),
        lv_color_hex(0xFFFFFF),
        lv_color_hex(0x000000),
        lv_color_hex(0xFF8000),
        lv_color_hex(0x8000FF),
        lv_color_hex(0x00FF80),
        lv_color_hex(0x808080)
    };

    idx++;
    idx %= sizeof(colors) / sizeof(colors[0]);

    lv_obj_set_style_bg_color(full_screen_obj, colors[idx], 0);

    /* 强制整屏刷新 */
    lv_obj_invalidate(lv_scr_act());
}

void ui_test_perf(void)
{
    lv_obj_t * scr = lv_scr_act();
    lv_obj_clean(scr);

    full_screen_obj = lv_obj_create(scr);
    lv_obj_remove_style_all(full_screen_obj);
    lv_obj_set_size(full_screen_obj, 320, 240);
    lv_obj_set_pos(full_screen_obj, 0, 0);

    lv_obj_set_style_bg_opa(full_screen_obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(full_screen_obj,
        lv_palette_main(LV_PALETTE_RED), 0);

    lv_timer_create(color_timer_cb, 300, nullptr);
}