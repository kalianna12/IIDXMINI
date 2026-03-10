#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
// 按键识别枚举
enum KeyAction {
    KEY_NONE,
    KEY_LEFT,    // A键
    KEY_DOWN,    // B键  
    KEY_RIGHT,   // C键
    KEY_UP,      // E键
    KEY_BACK,    // H键
    KEY_CONFIRM, // I键
    KEY_ENTER_MENU // I键快速点击4次
};

// 按键识别类
class KeyDetector {
private:
    // I键快速点击检测
    uint32_t i_click_times[4];  // 记录最近4次I键按下时间
    int i_click_count = 0;
    uint32_t last_i_press_time = 0;
    
    // 去抖动和冷却时间
    uint32_t last_action_time = 0;  // 上次触发动作的时间
    const uint32_t DEBOUNCE_DELAY = 50;  // 按键去抖动延迟(ms)
    const uint32_t ACTION_COOLDOWN = 100;  // 动作冷却时间(ms)
    
public:
    // 检测按键动作
    KeyAction detectKeyAction(bool a_pressed, bool b_pressed, bool c_pressed, 
                             bool d_pressed, bool e_pressed, bool f_pressed,
                             bool g_pressed, bool h_pressed, bool i_pressed) {
        
        uint32_t now = millis();
        
        // 检查动作冷却时间，避免抖动导致的连续触发
        if (now - last_action_time < ACTION_COOLDOWN) {
            // 更新状态但不处理动作
            updateLastStates(a_pressed, b_pressed, c_pressed, d_pressed, 
                           e_pressed, f_pressed, g_pressed, h_pressed, i_pressed);
            return KEY_NONE;
        }
        
        // 1. 检测I键快速点击4次进入菜单
        if (i_pressed && !i_pressed_last) {
            // 检查按键间隔，避免抖动
            if (now - last_i_press_time > DEBOUNCE_DELAY) {
                last_i_press_time = now;
                
                // 记录这次点击时间
                for (int i = 3; i > 0; i--) {
                    i_click_times[i] = i_click_times[i-1];
                }
                i_click_times[0] = now;
                i_click_count = min(i_click_count + 1, 4);
                
                // 检查是否在2秒内点击了4次
                if (i_click_count >= 4) {
                    uint32_t time_span = now - i_click_times[3];
                    if (time_span < 2000) {  // 2秒内4次点击
                        // 重置计数
                        i_click_count = 0;
                        last_action_time = now;
                        updateLastStates(a_pressed, b_pressed, c_pressed, d_pressed, 
                                       e_pressed, f_pressed, g_pressed, h_pressed, i_pressed);
                        return KEY_ENTER_MENU;
                    }
                }
            }
        }
        
        // 超时重置计数（如果超过2秒没有新点击）
        if (i_click_count > 0 && (now - i_click_times[0]) > 2000) {
            i_click_count = 0;
        }
        
        // 2. 检测单个按键（只在按下时响应，避免重复触发）
        KeyAction action = KEY_NONE;
        
        if (a_pressed && !a_pressed_last && (now - last_a_press_time > DEBOUNCE_DELAY)) {
            action = KEY_LEFT;
            last_a_press_time = now;
        } else if (b_pressed && !b_pressed_last && (now - last_b_press_time > DEBOUNCE_DELAY)) {
            action = KEY_DOWN;
            last_b_press_time = now;
        } else if (c_pressed && !c_pressed_last && (now - last_c_press_time > DEBOUNCE_DELAY)) {
            action = KEY_RIGHT;
            last_c_press_time = now;
        } else if (e_pressed && !e_pressed_last && (now - last_e_press_time > DEBOUNCE_DELAY)) {
            action = KEY_UP;
            last_e_press_time = now;
        } else if (h_pressed && !h_pressed_last && (now - last_h_press_time > DEBOUNCE_DELAY)) {
            action = KEY_BACK;
            last_h_press_time = now;
        } else if (i_pressed && !i_pressed_last && (now - last_i_single_press_time > DEBOUNCE_DELAY)) {
            action = KEY_CONFIRM;
            last_i_single_press_time = now;
        }
        
        // 更新状态
        updateLastStates(a_pressed, b_pressed, c_pressed, d_pressed, 
                       e_pressed, f_pressed, g_pressed, h_pressed, i_pressed);
        
        if (action != KEY_NONE) {
            last_action_time = now;
        }
        
        return action;
    }
    
private:
    // 按键状态追踪变量
    bool a_pressed_last = false;
    bool b_pressed_last = false;
    bool c_pressed_last = false;
    bool d_pressed_last = false;
    bool e_pressed_last = false;
    bool f_pressed_last = false;
    bool g_pressed_last = false;
    bool h_pressed_last = false;
    bool i_pressed_last = false;
    
    // 各按键最后按下时间（用于去抖动）
    uint32_t last_a_press_time = 0;
    uint32_t last_b_press_time = 0;
    uint32_t last_c_press_time = 0;
    uint32_t last_e_press_time = 0;
    uint32_t last_h_press_time = 0;
    uint32_t last_i_single_press_time = 0;
    
    // 更新所有按键的最后状态
    void updateLastStates(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h, bool i) {
        a_pressed_last = a;
        b_pressed_last = b;
        c_pressed_last = c;
        d_pressed_last = d;
        e_pressed_last = e;
        f_pressed_last = f;
        g_pressed_last = g;
        h_pressed_last = h;
        i_pressed_last = i;
    }
};

#endif