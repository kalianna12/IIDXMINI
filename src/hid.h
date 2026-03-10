#ifndef HID_H
#define HID_H

#include <Arduino.h>
#include "USBHID.h"
#include "USB.h"


extern USBHID HID;

extern float Mouse_Sensitivity;

// 导出所有按键状态（用于菜单）
extern bool hid_key_a_pressed;
extern bool hid_key_b_pressed;
extern bool hid_key_c_pressed;
extern bool hid_key_d_pressed;
extern bool hid_key_e_pressed;
extern bool hid_key_f_pressed;
extern bool hid_key_g_pressed;
extern bool hid_key_h_pressed;
extern bool hid_key_i_pressed;

void HID_KEYBOARD_Init();
void HID_KEYBOARD_LOOP();
void HID_MOUSE_LOOP();

#endif