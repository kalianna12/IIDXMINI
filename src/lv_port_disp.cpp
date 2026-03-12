#include "lv_port_disp.h"
/* 横屏逻辑分辨率 */
#define MY_DISP_HOR_RES 320
#define MY_DISP_VER_RES 240

/* 
 * 双缓冲
 * 每块缓冲区存 20 行像素：
 * 320 * 20 = 6400 个像素
 * lv_color_t 在 RGB565 下是 2 字节
 * 一块大约 12.5KB，两块约 25KB
 */
static lv_color_t buf1[MY_DISP_HOR_RES * 20];
static lv_color_t buf2[MY_DISP_HOR_RES * 20];
/* LVGL 的绘图缓冲区描述符 */
static lv_disp_draw_buf_t disp_buf;
/* 显示驱动描述符 */
static lv_disp_drv_t disp_drv;
static void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

void lv_port_disp_init(void)
{
    /* 1. 先初始化双缓冲 */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, MY_DISP_HOR_RES * 20);

    /* 2. 初始化显示驱动结构体 */
    lv_disp_drv_init(&disp_drv);

    /* 3. 绑定缓冲区和回调 */
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = my_flush_cb;

    /* 4. 设置逻辑分辨率 */
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /* 5. 注册显示驱动 */
    lv_disp_t * disp;
    disp = lv_disp_drv_register(&disp_drv);
}

static void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint16_t x = area->x1;
    uint16_t y = area->y1;
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(x, y, w, h);
    tft.writePixels((uint16_t *)color_p, (uint32_t)w * h, true, false);
    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}
