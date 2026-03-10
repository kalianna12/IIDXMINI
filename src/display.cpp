#include "display.h"
#include <math.h>    
#include "wifi_config.h" 
#include "ntp_time.h"
#include "weather.h"
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);  // 这是硬件SPI路径
U8G2_FOR_ADAFRUIT_GFX u8g2;//创建字库
//Adafruit_ST7789::Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
//                                 int8_t rst)
void display_init() {
    SPI.begin(12, -1, 11, 10);  // SCLK, MISO, MOSI, SS
    u8g2.begin(tft);
    
    tft.init(240, 320, SPI_MODE0); // 初始化显示屏，设置分辨率和SPI模式
    tft.setRotation(3); // 设置显示屏旋转方向
    tft.setSPISpeed(40000000);    // 设置SPI速度，单位为Hz，具体值根据显示屏和ESP32的性能调整
    tft.fillScreen(ST77XX_BLACK); // 清屏，设置背景色为黑色
    tft.setTextColor(ST77XX_WHITE); // 设置文本颜色为白色
    tft.setTextSize(2);
    tft.setCursor(0, 10);
    tft.println("ST7789 240x320 OK");
    delay(500);
}

void display_drawlines()
{static bool is_draw_lines=false;
  if(!is_draw_lines){
    tft.drawLine(0,40,320,40,ST77XX_WHITE);
    is_draw_lines=true;
  }
  else return;
}

void display_time() {
    if (!is_ntp_time_ready) return;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    // --- 1. 时间数值变化检测 ---
    bool hour_changed = (timeinfo.tm_hour != last_hour);
    bool min_changed  = (timeinfo.tm_min != last_min);
    bool sec_changed  = (timeinfo.tm_sec != last_sec);

    // --- 2. 冒号闪烁逻辑 (每 500ms 切换一次) ---
    bool current_colon_visible = (millis() % 1000 < 500);
    bool colon_needs_update = (current_colon_visible != last_colon_visible);

    tft.setTextSize(3); // 使用大号字体
    uint16_t bg_color = ST77XX_BLACK;
    uint16_t tx_color = ST77XX_WHITE;

    // --- 3. 分区域局部刷新 ---
    
    // 小时区域 (坐标 0, 10)
    if (hour_changed) {
        tft.fillRect(0, 10, 40, 25, bg_color); 
        tft.setCursor(0, 10);
        tft.setTextColor(tx_color);
        if (timeinfo.tm_hour < 10) tft.print("0");
        tft.print(timeinfo.tm_hour);
        last_hour = timeinfo.tm_hour;
    }

    // 第一个冒号 (坐标 42, 10)
    if (colon_needs_update || sec_changed) { // 秒变或闪烁时间到则刷
        tft.setCursor(42, 10);
        tft.setTextColor(current_colon_visible ? tx_color : bg_color);
        tft.print(":");
    }

    // 分钟区域 (坐标 55, 10)
    if (min_changed) {
        tft.fillRect(55, 10, 40, 25, bg_color);
        tft.setCursor(55, 10);
        tft.setTextColor(tx_color);
        if (timeinfo.tm_min < 10) tft.print("0");
        tft.print(timeinfo.tm_min);
        last_min = timeinfo.tm_min;
    }

    // 第二个冒号 (坐标 97, 10)
    if (colon_needs_update || sec_changed) {
        tft.setCursor(97, 10);
        tft.setTextColor(current_colon_visible ? tx_color : bg_color);
        tft.print(":");
    }

    // 秒钟区域 (坐标 110, 10)
    if (sec_changed) {
        tft.fillRect(110, 10, 40, 25, bg_color);
        tft.setCursor(110, 10);
        tft.setTextColor(tx_color);
        if (timeinfo.tm_sec < 10) tft.print("0");
        tft.print(timeinfo.tm_sec);
        last_sec = timeinfo.tm_sec;
    }

    last_colon_visible = current_colon_visible;
}

void display_weather()
{
  if(!is_weather_update_completed) return;
  if(last_city!=my_city||last_prov!=my_province){
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setForegroundColor(ST77XX_WHITE);
  u8g2.setFont(u8g2_font_wqy16_t_gb2312b);

  tft.fillRect(160,0,160,35,ST77XX_BLACK);
  u8g2.setCursor(200,30);
  u8g2.print(my_province);
  u8g2.print(" ");
  u8g2.print(my_city);
  last_city=my_city;
  last_prov=my_province;

  if(last_reporttime!=my_reporttime)
    {
      u8g2.setCursor(10,70);
      u8g2.print(my_weather);
      u8g2.print(" ");
      u8g2.print(my_temperature);
      u8g2.print(" ");
      u8g2.print(my_winddirection);
      u8g2.print(" ");
      u8g2.print(my_windpower);
      u8g2.print(" ");
      u8g2.print(my_humidity);
      u8g2.setCursor(10,90);
      u8g2.print(my_reporttime);
      last_reporttime=my_reporttime;
    }
  }

}

// ===================== 可调参数 =====================
static const uint32_t ANI_TOTAL_MS = 20000;
static const uint32_t TARGET_FPS   = 30;
static const uint32_t FRAME_US     = 1000000 / TARGET_FPS;

// 上方主动画：0~14s
static const uint32_t TOP_END_MS   = 14000;

// 阶段划分（都在 0~14s 内）
static const uint32_t T_GATHER_END = 3000;   // 0-3s：边缘彩球汇聚到中心并消失
static const uint32_t T_GROW_END   = 6000;   // 3-6s：中心变色球变大到半径40
static const uint32_t T_BOOM_END   = 10000;  // 6-10s：爆炸扩散（4s）
static const uint32_t T_LINE1_END  = 12000;  // 10-12s：左下->右上 0101 粗线（慢）
static const uint32_t T_LINE2_END  = 13000;  // 12-13s：左上->右下 1010 粗线（快）
static const uint32_t T_SETTLE_END = 14000;  // 13-14s：停住留画面

// 下方文字：KALIANNA 0~14s（1秒一个字）；14s 后 WELCOME 5s 逐字替换（含破碎）
static const uint32_t NAME_TO_WELCOME_MS = 14000;
static const uint32_t WELCOME_DUR_MS     = 5000;

// ===================== 小工具 =====================
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static inline float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

struct Vec2 { float x, y; };

static inline uint16_t colorWheel565(uint8_t pos) {
  // 0..255 -> 彩虹
  pos = 255 - pos;
  if (pos < 85) {
    return rgb565(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return rgb565(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return rgb565(pos * 3, 255 - pos * 3, 0);
  }
}

// ===================== 爆炸粒子 =====================
struct Particle {
  float x, y, vx, vy;
  uint8_t life;     // 帧寿命
  uint16_t col;
};

static const uint16_t COL_BG = ST77XX_BLACK;
static const uint16_t COL_BOX = rgb565(0, 60, 120);
static const uint16_t COL_BOX2 = rgb565(40, 140, 255);
static const uint16_t COL_WHITE = ST77XX_WHITE;

// ===================== 启动动画 =====================
bool display_startani() {
  static bool inited = false;
  static uint32_t t0_ms = 0;
  static uint32_t next_us = 0;

  // FPS
  static uint32_t fps_cnt = 0;
  static uint32_t fps_last_ms = 0;
  static uint16_t fps_show = 0;

  // 画面几何（适配 rotation=1，运行时取宽高）
  static int W, H;
  static int BOX_X, BOX_Y, BOX_W, BOX_H;
  static Vec2 C; // box center

  // 边缘汇聚彩球（数量多一点更“各种颜色”）
  static const int ORB_N = 14;
  static Vec2 orb_p[ORB_N];
  static Vec2 orb_v[ORB_N];
  static uint16_t orb_c[ORB_N];
  static bool orb_alive[ORB_N];

  // 中心变色球
  static float core_r = 0.0f;

  // 爆炸粒子
  static const int PNUM = 90;
  static Particle ps[PNUM];
  static bool boom_spawned = false;

  // 下方文字状态
  static const char* NAME = "KALIANNA"; // 8 chars
  static int name_shown = 0;

  static const char* WEL = "WELCOME";   // 7 chars
  static int wel_shown = 0;
  static bool wel_spawned_shatter = false;

  // 文字破碎粒子（只用于替换瞬间）
  static const int TPN = 60;
  static Particle tp[TPN];

  // FPS区位置（不顶格）
  static const int FPS_X = 8;
  static const int FPS_Y = 8;

  auto draw_box = [&]() {
    // 外框（大一些：取屏幕较短边的 70% 作为框尺寸）
    tft.drawRect(BOX_X, BOX_Y, BOX_W, BOX_H, COL_BOX);
    tft.drawRect(BOX_X+1, BOX_Y+1, BOX_W-2, BOX_H-2, rgb565(0,35,70));
    // 角装饰
    tft.drawFastHLine(BOX_X, BOX_Y, 16, COL_BOX2);
    tft.drawFastVLine(BOX_X, BOX_Y, 16, COL_BOX2);
    tft.drawFastHLine(BOX_X+BOX_W-16, BOX_Y, 16, COL_BOX2);
    tft.drawFastVLine(BOX_X+BOX_W-1,  BOX_Y, 16, COL_BOX2);

    tft.drawFastHLine(BOX_X, BOX_Y+BOX_H-1, 16, COL_BOX2);
    tft.drawFastVLine(BOX_X, BOX_Y+BOX_H-16, 16, COL_BOX2);
    tft.drawFastHLine(BOX_X+BOX_W-16, BOX_Y+BOX_H-1, 16, COL_BOX2);
    tft.drawFastVLine(BOX_X+BOX_W-1,  BOX_Y+BOX_H-16, 16, COL_BOX2);
  };

  auto clear_box_inside = [&]() {
    tft.fillRect(BOX_X+2, BOX_Y+2, BOX_W-4, BOX_H-4, COL_BG);
  };

  auto draw_fps = [&](uint32_t now_ms) {
    fps_cnt++;
    if (now_ms - fps_last_ms >= 1000) {
      fps_last_ms = now_ms;
      fps_show = fps_cnt;
      fps_cnt = 0;

      tft.fillRect(FPS_X, FPS_Y, 120, 22, COL_BG);
      tft.setCursor(FPS_X, FPS_Y);
      tft.setTextColor(COL_WHITE);
      tft.setTextSize(2);
      tft.print("FPS:");
      tft.print(fps_show);
    }
  };

  auto draw_name_area_bg = [&]() {
    // 底部条带区域
    int y = H - 34;
    tft.fillRect(0, y-2, W, 40, COL_BG);
  };

  auto draw_name_kalianna = [&](uint32_t elapsed) {
    // 1秒一个字，最多 8 个
    int target = (int)(elapsed / 1000) + 1;
    if (target > 8) target = 8;
    if (target > name_shown) name_shown = target;

    draw_name_area_bg();
    int y = H - 30;
    tft.setTextSize(2);
    tft.setTextColor(COL_WHITE);
    // 粗略居中
    int startX = (W - 8*12) / 2;
    tft.setCursor(startX, y);
    for (int i=0;i<name_shown;i++) tft.print(NAME[i]);
  };

  auto spawn_text_shatter = [&]() {
    // 在底部文字区域生成碎片向上冒出
    for (int i=0;i<TPN;i++) {
      float rx = (float)(W/2 + (rand()%120 - 60));
      float ry = (float)(H - 26 + (rand()%18 - 9));
      tp[i].x = rx;
      tp[i].y = ry;
      float ang = (float)(rand()%628) / 100.0f; // 0..6.28
      float spd = 0.8f + (rand()%120)/100.0f;
      tp[i].vx = cosf(ang) * spd;
      tp[i].vy = -fabsf(sinf(ang) * spd) - 0.6f; // 主要向上
      tp[i].life = 60;
      tp[i].col = colorWheel565((uint8_t)(rand()%256));
    }
  };

  auto draw_welcome = [&](uint32_t elapsed) {
    // 14s 后开始 5s 逐字 WELCOME，彩色
    uint32_t t = elapsed - NAME_TO_WELCOME_MS;
    float prog = clamp01((float)t / (float)WELCOME_DUR_MS);
    int target = (int)floorf(prog * 7.0f) + 1;
    if (target > 7) target = 7;
    if (target > wel_shown) wel_shown = target;

    // 先画碎片（作为替换动效）
    if (!wel_spawned_shatter) {
      wel_spawned_shatter = true;
      spawn_text_shatter();
    }

    // 更新碎片
    draw_name_area_bg();
    for (int i=0;i<TPN;i++) {
      if (!tp[i].life) continue;
      tp[i].x += tp[i].vx;
      tp[i].y += tp[i].vy;
      tp[i].vy += 0.03f; // 轻微重力
      tp[i].vx *= 0.99f;
      tp[i].vy *= 0.99f;
      tp[i].life--;
      tft.fillRect((int)tp[i].x, (int)tp[i].y, 2, 2, tp[i].col);
    }

    // 再写 WELCOME（彩色逐字）
    int y = H - 30;
    tft.setTextSize(2);
    int startX = (W - 7*12) / 2;
    tft.setCursor(startX, y);
    for (int i=0;i<wel_shown;i++) {
      // 每个字不同色，且随时间略变化
      uint8_t w = (uint8_t)((i*36 + (t/20)) & 255);
      tft.setTextColor(colorWheel565(w));
      tft.print(WEL[i]);
    }
  };

  auto thick_bits_line = [&](int x0, int y0, int x1, int y1, bool startWithOne,
                            float progress01, int thickness) {
    // 用 0/1 沿线绘制，progress01 控制画到哪里
    float px = lerp((float)x0, (float)x1, progress01);
    float py = lerp((float)y0, (float)y1, progress01);

    // 每隔 step 像素放一个字符
    const int step = 10;
    // 方向向量
    float dx = (float)(x1 - x0);
    float dy = (float)(y1 - y0);
    float len = sqrtf(dx*dx + dy*dy) + 0.001f;
    float ux = dx / len, uy = dy / len;
    // 垂直方向用于“加粗”
    float nx = -uy, ny = ux;

    // 当前已绘制长度
    float curLen = sqrtf((px - x0)*(px - x0) + (py - y0)*(py - y0));
    int count = (int)(curLen / step);

    tft.setTextSize(1);

    for (int i=0;i<=count;i++) {
      float t = (float)i * step / len;
      int bx = (int)lerp((float)x0, (float)x1, t);
      int by = (int)lerp((float)y0, (float)y1, t);

      char ch;
      bool bit = ((i & 1) == 0) ? startWithOne : !startWithOne;
      ch = bit ? '1' : '0';

      // 加粗：在法线方向偏移多条
      for (int k = -(thickness/2); k <= (thickness/2); k++) {
        int ox = bx + (int)(nx * k);
        int oy = by + (int)(ny * k);
        tft.setCursor(ox, oy);
        tft.setTextColor(COL_BOX2);
        tft.print(ch);
      }
    }
  };

  // ============ 初始化 ============
  if (!inited) {
    inited = true;
    t0_ms = millis();
    next_us = micros();

    fps_cnt = 0;
    fps_last_ms = millis();
    fps_show = 0;

    W = tft.width();
    H = tft.height();

    // 大框：取短边 70%，长边 70%（更大更显眼）
    int bw = (int)(W * 0.70f);
    int bh = (int)(H * 0.70f);
    BOX_W = bw;
    BOX_H = bh;
    BOX_X = (W - BOX_W) / 2;
    BOX_Y = (H - BOX_H) / 2;
    C = { BOX_X + BOX_W * 0.5f, BOX_Y + BOX_H * 0.5f };

    // 清屏 + 框
    tft.fillScreen(COL_BG);
    draw_box();

    // 初始化边缘彩球：随机落在框边缘一圈
    for (int i=0;i<ORB_N;i++) {
      // 选择边：0上 1下 2左 3右
      int edge = rand() & 3;
      float x, y;
      float margin = 6;
      if (edge == 0) {
        x = BOX_X + margin + (rand() % (BOX_W - (int)margin*2));
        y = BOX_Y + margin;
      } else if (edge == 1) {
        x = BOX_X + margin + (rand() % (BOX_W - (int)margin*2));
        y = BOX_Y + BOX_H - margin;
      } else if (edge == 2) {
        x = BOX_X + margin;
        y = BOX_Y + margin + (rand() % (BOX_H - (int)margin*2));
      } else {
        x = BOX_X + BOX_W - margin;
        y = BOX_Y + margin + (rand() % (BOX_H - (int)margin*2));
      }
      orb_p[i] = {x, y};
      // 速度指向中心
      float dx = C.x - x, dy = C.y - y;
      float d = sqrtf(dx*dx + dy*dy) + 0.001f;
      float spd = 1.4f + (rand()%70)/50.0f; // 1.4~2.8
      orb_v[i] = { dx/d * spd, dy/d * spd };
      orb_c[i] = colorWheel565((uint8_t)(i * 255 / ORB_N));
      orb_alive[i] = true;
    }

    // 爆炸粒子清空
    for (int i=0;i<PNUM;i++) ps[i].life = 0;
    boom_spawned = false;

    // 底部文字
    name_shown = 0;
    wel_shown = 0;
    wel_spawned_shatter = false;
    for (int i=0;i<TPN;i++) tp[i].life = 0;
  }

  // ============ 固定帧率 30FPS ============
  uint32_t now_us = micros();
  if ((int32_t)(now_us - next_us) < 0) return false;
  next_us += FRAME_US;

  uint32_t now_ms = millis();
  uint32_t elapsed = now_ms - t0_ms;

  if (elapsed >= ANI_TOTAL_MS) {
    // 结束：留最终画面（你也可以在这里清屏）
    return true;
  }

  // FPS（左上角）
  draw_fps(now_ms);

  // ============ 底部文字同步进行 ============
  if (elapsed < NAME_TO_WELCOME_MS) {
    draw_name_kalianna(elapsed);
  } else if (elapsed < NAME_TO_WELCOME_MS + WELCOME_DUR_MS) {
    draw_welcome(elapsed);
  } else {
    // WELCOME 完整显示后保持
    draw_name_area_bg();
    int y = H - 30;
    tft.setTextSize(2);
    int startX = (W - 7*12) / 2;
    tft.setCursor(startX, y);
    for (int i=0;i<7;i++) {
      tft.setTextColor(colorWheel565((uint8_t)(i*36)));
      tft.print(WEL[i]);
    }
  }

  // ============ 上方主动画（0~14s） ============
  if (elapsed < TOP_END_MS) {
    // 只清框内，不动外框
    clear_box_inside();

    // 根据阶段绘制
    if (elapsed < T_GATHER_END) {
      // 0-3s：各种颜色球从框边缘向中心汇聚，到了中心就消失
      for (int i=0;i<ORB_N;i++) {
        if (!orb_alive[i]) continue;
        orb_p[i].x += orb_v[i].x;
        orb_p[i].y += orb_v[i].y;

        float dx = orb_p[i].x - C.x;
        float dy = orb_p[i].y - C.y;
        float d = sqrtf(dx*dx + dy*dy);

        if (d < 10.0f) { // 到中心附近消失
          orb_alive[i] = false;
          continue;
        }

        // 球更大：半径 7~10
        int r = 7 + (i % 4);
        tft.fillCircle((int)orb_p[i].x, (int)orb_p[i].y, r, orb_c[i]);
        tft.drawCircle((int)orb_p[i].x, (int)orb_p[i].y, r+2, orb_c[i]);
      }

    } else if (elapsed < T_GROW_END) {
      // 3-6s：中心快速变色球逐渐变大到半径40
      float t = (elapsed - T_GATHER_END) / (float)(T_GROW_END - T_GATHER_END);
      t = clamp01(t);
      core_r = lerp(8.0f, 40.0f, t);

      // 颜色快速变化
      uint16_t col = colorWheel565((uint8_t)((elapsed / 8) & 255));
      uint16_t col2 = colorWheel565((uint8_t)((elapsed / 5) & 255));

      // 发光层：画几圈
      tft.drawCircle((int)C.x, (int)C.y, (int)(core_r + 6), col2);
      tft.drawCircle((int)C.x, (int)C.y, (int)(core_r + 3), col);
      tft.fillCircle((int)C.x, (int)C.y, (int)core_r, col);

    } else if (elapsed < T_BOOM_END) {
      // 6-10s：爆炸扩散 4s
      if (!boom_spawned) {
        boom_spawned = true;
        for (int i=0;i<PNUM;i++) {
          float ang = (float)i / PNUM * 6.28318f;
          float spd = 1.0f + (rand()%180)/60.0f; // 1~4
          ps[i].x = C.x;
          ps[i].y = C.y;
          ps[i].vx = cosf(ang) * spd;
          ps[i].vy = sinf(ang) * spd;
          ps[i].life = 120; // 约4秒（30fps*4=120）
          ps[i].col = colorWheel565((uint8_t)(rand()%256));
        }
      }

      for (int i=0;i<PNUM;i++) {
        if (!ps[i].life) continue;
        ps[i].x += ps[i].vx;
        ps[i].y += ps[i].vy;

        // 阻尼+轻微扩散
        ps[i].vx *= 0.985f;
        ps[i].vy *= 0.985f;

        // 粒子大小 2~3
        int sz = 2 + (i % 2);
        tft.fillRect((int)ps[i].x, (int)ps[i].y, sz, sz, ps[i].col);

        ps[i].life--;
      }

    } else if (elapsed < T_LINE1_END) {
      // 10-12s：左下->右上 绘制 0101 粗线（慢 2s）
      float t = (elapsed - T_BOOM_END) / (float)(T_LINE1_END - T_BOOM_END);
      t = clamp01(t);

      // 起点：框左下，终点：框右上（稍留边）
      int x0 = BOX_X + 10;
      int y0 = BOX_Y + BOX_H - 12;
      int x1 = BOX_X + BOX_W - 20;
      int y1 = BOX_Y + 12;

      // 厚度：5（更显眼）
      thick_bits_line(x0, y0, x1, y1, false /* startWithOne? */, t, 5);

    } else if (elapsed < T_LINE2_END) {
      // 12-13s：左上->右下 绘制 1010 粗线（快 1s）
      float t = (elapsed - T_LINE1_END) / (float)(T_LINE2_END - T_LINE1_END);
      t = clamp01(t);

      int x0 = BOX_X + 10;
      int y0 = BOX_Y + 12;
      int x1 = BOX_X + BOX_W - 20;
      int y1 = BOX_Y + BOX_H - 12;

      thick_bits_line(x0, y0, x1, y1, true /* startWithOne */, t, 4);

    } else {
      // 13-14s：保持一个“完成态”
      // 把两条线都画满，作为结束静帧
      int x0 = BOX_X + 10, y0 = BOX_Y + BOX_H - 12;
      int x1 = BOX_X + BOX_W - 20, y1 = BOX_Y + 12;
      thick_bits_line(x0, y0, x1, y1, false, 1.0f, 5);

      int a0 = BOX_X + 10, b0 = BOX_Y + 12;
      int a1 = BOX_X + BOX_W - 20, b1 = BOX_Y + BOX_H - 12;
      thick_bits_line(a0, b0, a1, b1, true, 1.0f, 4);

      // 中心点微亮一下
      tft.drawCircle((int)C.x, (int)C.y, 10, COL_BOX2);
      tft.fillCircle((int)C.x, (int)C.y, 3, COL_BOX2);
    }

    // 每帧重画框
    draw_box();
  }

  return false;
}