#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <SPI.h>
#include <lvgl.h>

// 定义显示屏引脚
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_MISO -1
#define TFT_CS   10
#define TFT_DC   14
#define TFT_RST  9
#define TFT_BL   21
#define LVGL_ON 1

// 创建显示屏对象
extern Adafruit_ST7789 tft;
extern U8G2_FOR_ADAFRUIT_GFX u8g2;
void display_init();
bool display_startani();
void display_time();
void display_weather();
void display_drawlines();

#endif