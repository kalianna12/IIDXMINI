#include "ntp_time.h"

// 定义 NTP 服务器地址和偏移
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600; // 东八区（北京时间）：8小时 * 3600秒
const int   daylightOffset_sec = 0;   // 中国没有夏令时，设为0

bool is_ntp_time_ready=false;
// 记录上一次的数值，用于对比刷新
int last_sec = -1;
int last_min = -1;
int last_hour = -1;
bool last_colon_visible = true;
uint32_t last_colon_ms = 0;

void init_ntp_time() {
    if(WiFi.status()!=WL_CONNECTED)
    {
        tft.println("ntp_time not ready");
        return;
    }
    // 配置 NTP 服务器和时区
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "ntp.aliyun.com");
    
    Serial0.println("Waiting for NTP time sync...");
    tft.println("NTP time sync...");
    
    // 等待时间同步（最多等待10秒）
    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
        Serial0.print(".");
        delay(1000);
        retry++;
    }

    if (retry < 10) {
        Serial0.println("\nTime synchronized successfully!");
        tft.println("ntp_time ready");
        // 打印当前时间
        Serial0.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
        tft.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
        is_ntp_time_ready=true;
    } else {
        Serial0.println("\nTime synchronization failed.");
        tft.println("ntp_time not ready");
    }
}

void print_local_time() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial0.println("Failed to obtain time");
        return;
    }
    // 格式化输出：2023-10-27 14:30:05
    Serial0.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}
/**
 * @brief 获取并返回格式化的时间字符串
 * @return String 格式如 "2026-02-11 10:40:23"
 */
String get_current_time_str() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time Error";
    }
    
    char timeStringBuff[20]; // 足够容纳 "yyyy-mm-dd hh:mm:ss\0"
    // strftime 是 C 标准库函数，专门用于格式化时间
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    return String(timeStringBuff);
}