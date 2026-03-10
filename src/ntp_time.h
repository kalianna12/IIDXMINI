#include <Arduino.h>
#include "time.h"
#include "wifi_config.h"
#include "display.h"

extern int last_sec ;
extern int last_min ;
extern int last_hour;
extern bool is_ntp_time_ready;
extern  bool last_colon_visible;
extern  uint32_t last_colon_ms ;

void init_ntp_time();
void print_local_time();
String get_current_time_str();