#include "weather.h"
#include <FreeRTOS.h>
#include "task.h"

void weatherupdate_init()
{
TaskHandle_t handle_weather_update;
static int param=30;
xTaskCreatePinnedToCore(weather_update,
    "weather_update",
    10240,
    (void*)&param,
    1,
    &handle_weather_update,
    0);
}

//ip 获取https://restapi.amap.com/v3/ip?ip=114.247.50.2&output=xml&key=<用户的key>
// {
//     "status": "1",
//     "info": "OK",
//     "infocode": "10000",
//     "province": "北京市",
//     "city": "北京市",
//     "adcode": "110000",
//     "rectangle": "116.0119343,39.66127144;116.7829835,40.2164962"
// }
// 建议将变量定义为全局，方便显示函数调用
String my_province = "";
String my_city = "";
String my_adcode = "";
String my_status = "";
String last_prov = "";
String last_city = "";


void weather_get_ip()
{
    if(WiFi.status()!=WL_CONNECTED) return;
    String url = "https://restapi.amap.com/v3/ip?key=3502ba6f649bb89a151eb5a362df8928";
    HTTPClient http;

    http.setTimeout(5000);
    http.begin(url);
    int httpCode=http.GET();

    if(httpCode==HTTP_CODE_OK)
    {   
        // 将响应内容直接解析，减少中间 String 变量对 SRAM 的占用
        // 使用 DynamicJsonDocument 替代 JsonDocument 以利用堆内存
        JsonDocument doc;
        // 使用 Stream 解析可以极大节省内存，不需要 getString() 存入大的 String 
        DeserializationError error = deserializeJson(doc, http.getStream());

        if(!error){
            my_status=doc["status"].as<String>();
            if(my_status=="1"){
            my_province=doc["province"].as<String>();;//得到具体的地理位置
            my_city=doc["city"].as<String>();;
            my_adcode=doc["adcode"].as<String>();;
            }            
        }
        else Serial0.println("status error");
    }
    else Serial0.printf("HTTP get error: %s\n",http.errorToString(httpCode).c_str());
    http.end();
}


 //https://restapi.amap.com/v3/weather/weatherInfo?city=110101&key=<用户key>
//     {
//     "status": "1",
//     "count": "1",
//     "info": "OK",
//     "infocode": "10000",
//     "lives": [
//         {
//             "province": "江西",
//             "city": "南昌市",
//             "adcode": "360100",
//             "weather": "阴",
//             "temperature": "19",
//             "winddirection": "北",
//             "windpower": "≤3",
//             "humidity": "75",
//             "reporttime": "2026-02-15 19:38:21",
//             "temperature_float": "19.0",
//             "humidity_float": "75.0"
//         }
//     ]
// }
String my_status2="";
String my_weather="";
String my_temperature="";
String my_winddirection="";
String my_windpower="";
String my_humidity="";
String my_reporttime="";
String last_reporttime="";
void weather_get_local_weather(){
    if(WiFi.status()!=WL_CONNECTED) return;
   
    String url = "https://restapi.amap.com/v3/weather/weatherInfo?city="+my_adcode+"&key=3502ba6f649bb89a151eb5a362df8928";
    HTTPClient http;

    http.setTimeout(5000);
    http.begin(url);
    int httpCode=http.GET();

    if(httpCode==HTTP_CODE_OK)
    {   
        // 将响应内容直接解析，减少中间 String 变量对 SRAM 的占用
        // 使用 DynamicJsonDocument 替代 JsonDocument 以利用堆内存
        JsonDocument doc;
        // 使用 Stream 解析可以极大节省内存，不需要 getString() 存入大的 String 
        DeserializationError error = deserializeJson(doc, http.getStream());

        if(!error){
            my_status2=doc["status"].as<String>();
            if(my_status2=="1")
            {
                JsonObject live = doc["lives"][0]; 
                
                if (!live.isNull()) { // 增加判空保护
                    my_weather = live["weather"].as<String>();
                    my_temperature = live["temperature"].as<String>();
                    my_winddirection = live["winddirection"].as<String>();
                    my_windpower = live["windpower"].as<String>();
                    my_humidity = live["humidity"].as<String>();
                    my_reporttime = live["reporttime"].as<String>();
                    Serial0.println("Weather parsed successfully: " + my_weather);
                } else {
                    Serial0.println("lives array is empty!");
                }
            }          
        }
        else Serial0.println("status error");
    }
    else Serial0.printf("HTTP get error: %s\n",http.errorToString(httpCode).c_str());
    http.end();
}


bool is_weather_update_completed=false;
void weather_update(void* param)
{
    int interval_mins = *(int*)param;
    for (;;) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial0.println("[Weather] Starting update...");
            weather_get_ip(); // 内部获取 IP
            if(my_adcode!=""){
                weather_get_local_weather();           
                // 标记已完成一次获取，display 收到此信号后开始绘图
                is_weather_update_completed = true;            
                Serial0.println("[Weather] Update finished.");
            }
        }
        
        // 这里的延迟非常重要：将 CPU 权限交还给系统
        // interval_mins 分钟更新一次。如果获取失败，可以缩短这个时间重试。
        vTaskDelay(pdMS_TO_TICKS(interval_mins * 60 * 1000));
    }
}
