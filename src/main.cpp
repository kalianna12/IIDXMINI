#include <Arduino.h>
#include "wifi_config.h"
#include "ntp_time.h"
#include "display.h"
#include "hid.h"
#include <FreeRTOS.h>
#include "task.h"
#include "weather.h"
#include "e6b2_cwz6c.h"
#include "my_pn532.h"
#include "led.h"
#include "menu.h"


inline bool every_ms(uint32_t &last, uint32_t interval)
{
    uint32_t now = millis();
    if (now - last >= interval) {
        last = now;
        return true;
    }
    return false;
}

KeyDetector detector;
void KeyDetect();

// put function declarations here:
bool config_mode = false;
bool isnot_reset_display = true;
uint32_t last_hid = 0;
uint32_t last_ec11 = 0;
uint32_t last_display = 0;
uint32_t last_weather_update = 0;
uint32_t last_e6b2 = 0;
uint32_t last_mouse=0;
uint32_t last_display_e6b2=0;
uint32_t last_pn532_detect=0;
uint32_t last_led=0;

void wifi_config_setup();
void speed_benchmark();
void display_ani_process();
void hid_init_test();
void display_process();
void display_ram();

void setup() {

    HID_KEYBOARD_Init();

    Serial.begin(115200);
    Serial0.begin(115200);
    delay(100); // 等待串口稳定
    Serial0.println("Serial initialized.");
    
    display_init();
    Serial0.println("Display initialized.");

    e6b2_cwz6c_init();

    //PN532_Init();

    hid_init_test();

    wifi_config_setup();

    init_ntp_time();

    if(WiFi.status()==WL_CONNECTED) weatherupdate_init();
    LED_Init();

    delay(1000); // 等待一切稳定
    
       
}

void loop() {
    if (config_mode) {
        wifi_config_handle(); // 持续处理网页请求
    }
    else{
    //EVERY_MS(3000){Serial.println(get_current_time_str());}
    
    // 按键检测
    KeyDetect();
    
    display_ani_process(); // 启动动画&display显示
    if(every_ms(last_e6b2,5)){e6b2_cwz6c_process();}
    if(every_ms(last_mouse, 1)){HID_MOUSE_LOOP();}//鼠标轮询1ms
    if(every_ms(last_hid, 1)){HID_KEYBOARD_LOOP();}//键盘轮询1ms
    if(every_ms(last_led, 50)){LED_LOOP();}//LED轮询50ms
    //if(every_ms(last_display_e6b2, 100)){display_e6b2();}//显示参数轮询100ms
    //if(every_ms(last_pn532_detect, 200)){PN532_Process();}
    }
    
    
}

// 下面是函数实现
void wifi_config_setup()

{
  Preferences p;
    p.begin("wifi_store", false);
    String ssid = p.getString("ssid", "");
    String pass = p.getString("pass", "");
    p.end();

    if (ssid != "") {
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial0.print("Connecting to WiFi...");
        tft.println("Connecting to WiFi...");
        int retry = 0;
        while (WiFi.status() != WL_CONNECTED && retry < 20) {
            delay(500);
            Serial0.print(".");
            retry++;
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial0.println("\nWiFi Failed. Starting AP Mode.");
        tft.println("\nWiFi Failed. Starting AP Mode.");
        wifi_config_init(); // 开启网页配网
        config_mode = true;
    } else {
        Serial0.println("\nWiFi Connected!");
        tft.println("WiFi Connected!");
        tft.println("IP: " + WiFi.localIP().toString());
        init_ntp_time(); // 初始化 NTP 时间同步
    }
}

void display_ani_process()
{
    static bool done = true; // true跳过动画
    if (!done) {
    done = display_startani(); // 返回 true 表示8秒结束
    } else {
        if(isnot_reset_display)
        {
            tft.fillScreen(ST77XX_BLACK); // 清屏，设置背景色为黑色
            isnot_reset_display = false;
            delay(50);
        }
        display_process();       // 你的正常显示
    }
}

void hid_init_test()
{
    if (HID.ready()) {
        Serial0.println("HID ready");
        tft.setTextColor(ST77XX_WHITE);
        tft.println("HID ready");
        
    } else {
        tft.setTextColor(ST77XX_WHITE);
        tft.println("HID not ready");
        Serial0.println("HID not ready");
    }
}

void display_process() { 
  display_drawlines();
  if(every_ms(last_display,100)) display_time();
  display_weather();
  //display_ram();
}
void display_ram()
{   
// getFreeHeap	内部 SRAM (512KB)	~256KB (健康)	决定程序是否会因为逻辑复杂而崩溃。
// getFreePsram	外部 PSRAM (8MB)	~8.3MB (极充裕)	决定你能否处理巨型图片、长字符串或流畅绘图。
// getFreeSketchSpace	Flash 存储 (16MB)	视分区表而定	决定你能不能塞进更大、更全的字体文件。
   tft.fillRect(0,60,320,80,ST77XX_BLACK);
   tft.setCursor(0,60);
   tft.setTextSize(2);
   tft.setTextColor(ST77XX_WHITE); 
   tft.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
   tft.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());  
   tft.printf("psram free: %d bytes\n", ESP.getFreePsram());
   tft.printf("flash free: %d bytes\n", ESP.getFreeSketchSpace());

}

void KeyDetect()
{
   // 在loop中调用
KeyAction action = detector.detectKeyAction(
    hid_key_a_pressed,   // 传入A键状态
    hid_key_b_pressed,   // 传入B键状态
    hid_key_c_pressed,   // 传入C键状态
    hid_key_d_pressed,   // 传入D键状态
    hid_key_e_pressed,   // 传入E键状态
    hid_key_f_pressed,   // 传入F键状态
    hid_key_g_pressed,   // 传入G键状态
    hid_key_h_pressed,   // 传入H键状态
    hid_key_i_pressed    // 传入I键状态
);

// 根据返回值处理
switch (action) {
    case KEY_NONE:
        // 没有检测到任何动作
        break;
        
    case KEY_LEFT:
        Serial0.println("A键: 左方向");
        // 在菜单中向左移动，或其他左方向操作
        break;
        
    case KEY_DOWN:
        Serial0.println("B键: 下方向");
        // 在菜单中向下移动，或其他下方向操作
        break;
        
    case KEY_RIGHT:
        Serial0.println("C键: 右方向");
        // 在菜单中向右移动，或其他右方向操作
        break;
        
    case KEY_UP:
        Serial0.println("E键: 上方向");
        // 在菜单中向上移动，或其他上方向操作
        break;
        
    case KEY_BACK:
        Serial0.println("H键: 返回");
        // 返回上一级菜单，或取消操作
        break;
        
    case KEY_CONFIRM:
        Serial0.println("I键: 确认");
        // 确认选择，或执行当前操作
        break;
        
    case KEY_ENTER_MENU:
        Serial0.println("I键快速点击4次: 进入菜单");
        // 切换到菜单界面
        break;
        
    default:
        Serial0.println("未知按键动作");
        break;
}
}