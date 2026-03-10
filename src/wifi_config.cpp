#include "wifi_config.h"
#include "display.h"

WebServer server(80);
Preferences prefs;

// 简单的 HTML 页面字符串
const char* config_html = R"rawliteral(
<!DOCTYPE html><html>
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input { padding: 10px; width: 80%; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }
    button { padding: 10px 20px; background-color: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; }
</style>
</head>
<body>
    <h2>ESP32 设备配网</h2>
    <form action="/save" method="POST">
        <input type="text" name="ssid" placeholder="WiFi 名称" required><br>
        <input type="password" name="pass" placeholder="WiFi 密码"><br>
        <button type="submit">保存并连接</button>
    </form>
</body></html>
)rawliteral";

// 处理根目录访问
void handleRoot() {
    server.send(200, "text/html", config_html);
}

// 处理保存请求
void handleSave() {
    if (server.hasArg("ssid")) {
        String s = server.arg("ssid");
        String p = server.arg("pass");
        
        // 保存到 NVS (断电不丢失)
        prefs.begin("wifi_store", false);//读写模式
        prefs.putString("ssid", s);
        prefs.putString("pass", p);
        prefs.end();
        Serial0.println("Saved WiFi credentials: " + s);
        Serial0.println("Saved WiFi password: " + p);
        Serial0.println("Restarting to connect...");

        server.send(200, "text/html", "<html><body><h3>已收到信息，设备正在重启连接...</h3></body></html>");
        delay(2000);
        ESP.restart(); // 重启后，逻辑会自动进入“尝试连接”阶段
    }
}

void wifi_config_init() {
    Serial0.println("Starting Config AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32", "12345678");
    Serial0.println("WIFI NAME:ESP32     DEFAULT PASSWORD:12345678");
    
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
    
    Serial0.print("AP IP address: ");
    Serial0.println(WiFi.softAPIP());
    Serial0.println("Open a web browser and navigate to the above IP to configure WiFi.");
}

void wifi_config_handle() {
    static bool is_wifi_display=false;
    if(!is_wifi_display){
        static bool is_wifi_fillscreen=false;
        if(!is_wifi_fillscreen)
        {tft.fillScreen(ST77XX_BLACK);
            is_wifi_fillscreen=true;
        }
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(10,0);
        tft.setTextSize(2);
        tft.println("ESP32 12345678");
        tft.println("search IP address to set wifi");
        is_wifi_display=true;
    }
    server.handleClient();
}