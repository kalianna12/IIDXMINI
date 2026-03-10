#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

extern bool is_weather_update_completed;
extern String my_province ;
extern String my_city ;
extern String my_adcode ;
extern String my_status ;
extern String last_prov ;
extern String last_city ;
extern String my_status2;
extern String my_weather;
extern String my_temperature;
extern String my_winddirection;
extern String my_windpower;
extern String my_humidity;
extern String my_reporttime;
extern String last_reporttime;

void weather_get_ip();//在这里获取到ip位置和天气并更改要用到的全局变量（在变化的时候才更改）
void weather_get_local_weather();
void weatherupdate_init();//创建多任务，防止网络程序中断其他程序运行
void weather_update(void* param);


#endif