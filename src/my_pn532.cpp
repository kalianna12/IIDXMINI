#include "my_pn532.h"
// If using the breakout with SPI, define the pins for SPI communication.
// #define PN532_SCK  (48)
// #define PN532_MOSI (-1)
// #define PN532_SS   (27)
// #define PN532_MISO (-1)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
// #define PN532_IRQ   (-1)
// #define PN532_RESET (-1)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// Or use hardware Serial:
//Adafruit_PN532 nfc(PN532_RESET, &Serial1);

#define PN532_SDA 47
#define PN532_SCL 48
#define PN532_IRQ -1
#define PN532_RST -1
Adafruit_PN532 nfc(PN532_IRQ, PN532_RST);
// Aime 官方默认密钥 (用于认证 Block 1)
uint8_t AIME_KEY[6] = { 0x57, 0x43, 0x43, 0x46, 0x76, 0x32 };
// 存储读取到的卡号字符串
char global_card_id[17];

void PN532_Init()
{
    if(!Wire.begin(PN532_SDA,PN532_SCL)) Serial0.println("PN532 init failed");
    //bool begin(int sda, int scl, uint32_t frequency = 0U)
    if(nfc.begin()) Serial0.println("nfc ready");else Serial.println("nfc not ready");
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
        Serial0.print("Didn't find PN53x board");
    }
    Serial0.print("Found chip PN5"); Serial0.println((versiondata>>24) & 0xFF, HEX);
    Serial0.print("Firmware ver. "); Serial0.print((versiondata>>16) & 0xFF, DEC);
    Serial0.print('.'); Serial0.println((versiondata>>8) & 0xFF, DEC);

    if(nfc.SAMConfig())//读卡模式
    Serial0.println("Waiting for a card");
    else Serial0.println("读卡模式设置失败");
}

bool card_read=false;
// void read_felica()
// {
//     uint8_t idm[8];
//     uint8_t pmm[8];
    
//     if (nfc.felica_Polling(0xFFFF, 0x00, idm, pmm, nullptr, 200) == 1) {
//         if(!card_read){
//         Serial0.print("AIME ID: ");
//         for (int i = 0; i < 8; i++) {
//             if (idm[i] < 0x10) Serial0.print("0");
//             Serial0.print(idm[i], HEX);
//         }
//         Serial0.println();
//         card_read=true;
//         delay(100);
//         }
//     } 
//     else card_read=false;
// }
void read_mifare()
{
    uint8_t success;
    uint8_t uid[]={0,0,0,0,0,0,0,0};
    uint8_t uidlength;

    success=nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A,uid,&uidlength,500);
    if (success) {
        if(!card_read){
            Serial0.println("--- 发现 A 类卡 ---");
            if (uidlength == 4) Serial0.println("类型: Mifare Classic");
            if (uidlength == 7) Serial0.println("类型: Mifare Ultralight");
            Serial0.print("uid: ");
            for (int i = 0; i < 8; i++) {
                if (uid[i] < 0x10) Serial0.print("0");
                Serial0.print(uid[i], HEX);
            }
            card_read=true;
            delay(100);
        }
  }
  else card_read=false;
  
}

void PN532_Process()
{
    // read_felica();
    // if(!card_read) read_mifare();
}