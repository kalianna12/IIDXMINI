#include "hid.h"
#include "display.h"
#include "e6b2_cwz6c.h"
// ====================鼠标部分======================

float Mouse_Sensitivity = 1;


// =======================================================
// 1) 你的 3x3 矩阵引脚（按实际接线改）
//    规则：rowPins -> 接 R1~R3，colPins -> 接 C1~C3
// =======================================================
static const uint8_t ROWS = 3;
static const uint8_t COLS = 3;

// 示例引脚：你务必按你实际接线改掉
uint8_t rowPins[ROWS] = {15, 16, 17};   // R1,R2,R3
uint8_t colPins[COLS] = {7, 6, 5};       // C1,C2,C3

USBHID HID;

// 导出所有按键状态
bool hid_key_a_pressed = false;
bool hid_key_b_pressed = false;
bool hid_key_c_pressed = false;
bool hid_key_d_pressed = false;
bool hid_key_e_pressed = false;
bool hid_key_f_pressed = false;
bool hid_key_g_pressed = false;
bool hid_key_h_pressed = false;
bool hid_key_i_pressed = false;

// =======================================================
// 2) 键盘布局映射（R行 x C列）
//    当前映射：
//    A B C
//    D E F
//    G H I
// =======================================================
static const uint8_t keymap[ROWS][COLS] = {
    {HID_KEY_A, HID_KEY_B, HID_KEY_C},
    {HID_KEY_D, HID_KEY_E, HID_KEY_F},
    {HID_KEY_G, HID_KEY_H, HID_KEY_I}
};

// =======================================================
// 双报告：Report ID 定义
// =======================================================
static const uint8_t RID_BOOT_6KRO = 1;  
static const uint8_t RID_NKRO      = 2;  

// =======================================================
// HID 报告描述符
// =======================================================
static const uint8_t hid_report_desc[] = {
    // ======== 键盘部分 (RID 1 & 2) ========
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 
    0x85, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x01, 0x05, 0x07, 0x19, 0x00, 0x29, 0xFF, 0x15, 0x00, 0x25, 0xFF, 0x75, 0x08, 0x95, 0x06, 0x81, 0x00, 
    0xC0, 

    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 
    0x85, 0x02, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x01, 0x05, 0x07, 0x19, 0x00, 0x29, 0xFF, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x96, 0x00, 0x01, 0x81, 0x02, 
    0xC0,

    // ======== 鼠标部分 (RID 3) ========
    0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 
    0x85, 0x03,                // REPORT ID 3
    0x09, 0x01, 0xA1, 0x00, 
    0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x95, 0x03, 0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01, 
    0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06, 
    0xC0, 0xC0
};

// ===== HID 设备类 =====
class CalcHIDDevice : public USBHIDDevice {
public:
    uint16_t _onGetDescriptor(uint8_t* buffer) override {
        memcpy(buffer, hid_report_desc, sizeof(hid_report_desc));
        return sizeof(hid_report_desc);
    }
};

static CalcHIDDevice g_hid_dev;

// =======================================================
static volatile uint8_t g_protocol = 1;

static void hid_event_cb(void* arg, esp_event_base_t base, int32_t id, void* data)
{
    (void)arg;
    (void)base;

    if (id == ARDUINO_USB_HID_SET_PROTOCOL_EVENT) {
        auto* e = (arduino_usb_hid_event_data_t*)data;
        g_protocol = e->set_protocol.protocol;
    }
}

// =======================================================
static bool stable[ROWS][COLS] = {0};
static uint8_t dbcnt[ROWS][COLS] = {0};
static const uint8_t DB_N = 3;

// =======================================================
static void matrix_init()
{
    for (int r = 0; r < ROWS; r++) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }

    for (int c = 0; c < COLS; c++) {
        pinMode(colPins[c], INPUT_PULLUP);
    }
}

// =======================================================
static void scan_raw(bool raw[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++) {

        digitalWrite(rowPins[r], LOW);
        delayMicroseconds(50);

        for (int c = 0; c < COLS; c++) {
            raw[r][c] = (digitalRead(colPins[c]) == LOW);
        }

        digitalWrite(rowPins[r], HIGH);
    }
}

// =======================================================
static void send_boot_6kro()
{
    uint8_t rpt[8] = {0};
    int idx = 0;

    for (int r = 0; r < ROWS && idx < 6; r++) {
        for (int c = 0; c < COLS && idx < 6; c++) {
            if (stable[r][c]) {
                rpt[2 + idx] = keymap[r][c];
                idx++;
            }
        }
    }

    HID.SendReport(RID_BOOT_6KRO, rpt, sizeof(rpt));
}

// =======================================================
static uint8_t nkro_rpt[34] = {0};

static inline void nkro_set_bit(uint8_t keycode, bool down)
{
    uint8_t byteIndex = 2 + (keycode >> 3);
    uint8_t mask = 1u << (keycode & 7);

    if (down)
        nkro_rpt[byteIndex] |= mask;
    else
        nkro_rpt[byteIndex] &= (uint8_t)~mask;
}

static void rebuild_nkro_bitmap_from_stable()
{
    for (int i = 2; i < 34; i++)
        nkro_rpt[i] = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (stable[r][c]) {
                nkro_set_bit(keymap[r][c], true);
            }
        }
    }
}

static void send_nkro()
{
    rebuild_nkro_bitmap_from_stable();
    HID.SendReport(RID_NKRO, nkro_rpt, sizeof(nkro_rpt));
}

// =======================================================
static void send_by_protocol()
{
    if (g_protocol == 0)
        send_boot_6kro();
    else
        send_nkro();
}

// =======================================================
static void matrix_process()
{
    static bool raw[ROWS][COLS];
    scan_raw(raw);

    bool changed = false;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {

            bool cur = raw[r][c];
            bool old = stable[r][c];

            if (cur == old) {
                dbcnt[r][c] = 0;
            } else {
                if (++dbcnt[r][c] >= DB_N) {
                    dbcnt[r][c] = 0;
                    stable[r][c] = cur;
                    changed = true;
                }
            }
        }
    }

    // 更新所有按键状态（用于菜单系统）
    hid_key_a_pressed = stable[0][0];  // A
    hid_key_b_pressed = stable[0][1];  // B
    hid_key_c_pressed = stable[0][2];  // C
    hid_key_d_pressed = stable[1][0];  // D
    hid_key_e_pressed = stable[1][1];  // E
    hid_key_f_pressed = stable[1][2];  // F
    hid_key_g_pressed = stable[2][0];  // G
    hid_key_h_pressed = stable[2][1];  // H
    hid_key_i_pressed = stable[2][2];  // I

    if (changed && HID.ready()) {
        send_by_protocol();
    }
}

// =======================================================
void HID_KEYBOARD_Init()
{
    USBHID::addDevice(&g_hid_dev, sizeof(hid_report_desc));
    HID.onEvent(hid_event_cb);
    HID.begin();
    
    USB.begin();
    delay(100);

    matrix_init();

    if (HID.ready()) {       
        send_by_protocol();
    } else {       
    }
}

// =======================================================
void HID_KEYBOARD_LOOP()
{
    matrix_process();
}

void HID_MOUSE_LOOP()
{
    if (encoder_delta != 0) {
        int32_t target_x = (int32_t)(encoder_delta * Mouse_Sensitivity);
        int8_t x_val = (int8_t)constrain(target_x, -127, 127);

        // 数据包结构：[Buttons(1 byte), X(1 byte), Y(1 byte), Wheel(1 byte)]
        // 总长度 4 字节（对应描述符中鼠标部分的 Input 长度）
        uint8_t mouse_rpt[4] = {0, 0, 0, 0}; 
        mouse_rpt[1] = (uint8_t)x_val; 

        if (HID.ready()) {
            // 直接发送给 ID 3
            HID.SendReport(3, mouse_rpt, sizeof(mouse_rpt));
        }

        encoder_delta = 0; 
    }
}

