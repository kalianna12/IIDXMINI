#include "e6b2_cwz6c.h"
#include "display.h"

Encoder* myEnc = nullptr; // 初始为空指针
long oldPosition = -999;
long encoder_delta = 0;

void e6b2_cwz6c_init() {
    // 在系统一切稳定后（setup中）再创建对象
    myEnc = new Encoder(1, 2); 
    Serial0.println("E6B2 Encoder initialized on GPIO 1 & 2");
}

void e6b2_cwz6c_process(){
    if (myEnc == nullptr) return; // 没初始化前不执行逻辑

    long newPosition = myEnc->read();
    if (newPosition != oldPosition) {
        encoder_delta=newPosition-oldPosition;
        oldPosition = newPosition;
        
    }
}

void display_e6b2()
{
    // 你的屏幕刷新逻辑
        tft.fillRect(0,100,100,40,ST77XX_BLACK);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(2);
        tft.setCursor(0,100);
        tft.println(oldPosition);
        tft.print(encoder_delta);
        Serial0.print(encoder_delta);
}