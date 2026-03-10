#include "led.h"
#define LED_PIN 8
#define NUM_LEDS 9

CRGB leds[NUM_LEDS];
CRGB current_color = CRGB::Red;
void LED_Init() {
    FastLED.addLeds<WS2812,LED_PIN,GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(50);
}

void rainbowEffect() {
    static uint8_t hue = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 255 / NUM_LEDS), 255, 255);  // 彩虹色
    }
    FastLED.show();
    hue++;
}

void LED_LOOP() {
    rainbowEffect();  // 调用灯效函数
}