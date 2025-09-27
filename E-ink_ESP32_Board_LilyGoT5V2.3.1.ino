//This version is for LilyGo T5 2.13 E-ink ESP32 Board

// include library, include base class, make path known
#include <GxEPD.h>
#include "SPI.h"

// for prov
#include "sdkconfig.h"
#if CONFIG_ESP_WIFI_REMOTE_ENABLED
#error "WiFiProv is only supported in SoCs with native Wi-Fi support"
#endif

#include "WiFiProv.h"
#include "WiFi.h"

//  由于屏幕有多个版本，如下载程序后出现花屏请将下面4个头文件都测试一遍！
#include <GxDEPG0213BN/GxDEPG0213BN.h>


// FreeFonts from Adafruit_GFX
#include <Fonts/TomThumb.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <trophyBitmap.h>
#include <waitforride.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17



#define BUTTON_PIN 39

//保存用户数据的结构体
struct UserData {
  String username;
  int currentYear;
  double totalDistanceCurrentYear;
  double targetDistanceCurrentYear;
  double biggestClimb;
  double longestDistance;
  String lastActivityDate;
  double lastActivityDistance;
  String lastActivityAVGSpeed;
  String lastActivityMovingTime;
  String lastUpdateFromStrava;
  
};
UserData userData;

GxIO_Class io(SPI, /*CS=5*/ ELINK_SS, /*DC=*/ELINK_DC, /*RST=*/ELINK_RESET);
GxEPD_Class display(io, /*RST=*/ELINK_RESET, /*BUSY=*/ELINK_BUSY);

SPIClass sdSPI(VSPI);


int startX = 9, startY = 20;

// API 地址（check https://stravaapp.uxengineer.top, if you don't have a server to transfer Strava Datas）
const char* serverName = "https://api.uxengineer.top/api/device/#YOURDEVICEID#/dashboard-data";
// 分配足够的 JSON 内存
  StaticJsonDocument<4096> doc;
// 记录按钮上一次的状态，避免重复触发
int lastButtonState = HIGH;

// #define USE_SOFT_AP // Uncomment if you want to enforce using the Soft AP method instead of BLE
const char *pop = "abcd1234";           // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
const char *service_name = "PROV_123";  // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
const char *service_key = NULL;         // Password used for SofAP method (NULL = no password needed)
bool reset_provisioned = true;          // When true the library will automatically delete previously provisioned data.
// WARNING: SysProvEvent is called from a separate FreeRTOS task (thread)!
void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      {
        IPAddress ip = IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr);
        Serial.print("\nConnected IP address : ");
        Serial.println(ip);

        
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("Subnet Mask: ");
        Serial.println(WiFi.subnetMask());
        
        // ✅ WiFi 成功连上后，自动拉取数据
        getUserInfo();

        break;
      }
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: Serial.println("\nDisconnected. Connecting to the AP again... "); break;
    case ARDUINO_EVENT_PROV_START: Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app"); break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *)sys_event->event_info.prov_cred_recv.password);
        
        break;
      }
    case ARDUINO_EVENT_PROV_CRED_FAIL:
      {
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if (sys_event->event_info.prov_fail_reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) {
          Serial.println("\nWi-Fi AP password incorrect");
        } else {
          Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
        }
        break;
      }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS: Serial.println("\nProvisioning Successful"); break;
    case ARDUINO_EVENT_PROV_END: Serial.println("\nProvisioning Ends"); break;
    default: break;
  }
}

//从API获取数据
void getUserInfo() {
  

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Sending request...");
    HTTPClient http;
    http.begin(serverName);
    Serial.print("FREERAM");
    Serial.println(ESP.getFreeHeap());
    int httpResponseCode = http.GET();
    Serial.print("FREERAM");
    Serial.print(ESP.getFreeHeap());
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Payload: " + payload);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      userData.username = doc["data"]["stravaAccount"]["username"].as<String>();
      //currentYear
      userData.currentYear = doc["data"]["yearlyRideDistance"]["year"];
      Serial.println("userData.currentYear");
      Serial.println(userData.currentYear);
      Serial.println("------------------");
      Serial.println("currentYear"+userData.currentYear);
      Serial.println("------------------");
      //currentYearDistance
      userData.totalDistanceCurrentYear = doc["data"]["yearlyRideDistance"]["totalDistanceKm"];
      Serial.println("userData.totalDistanceCurrentYear");
      Serial.println(userData.totalDistanceCurrentYear);
      Serial.println("------------------");
      Serial.println("userData.totalDistanceCurrentYear"+String(userData.totalDistanceCurrentYear));
      Serial.println("------------------");
      //target
      userData.targetDistanceCurrentYear = doc["data"]["yearRideTarget"]["targetDistance"];
      Serial.println("------------------");
      Serial.println("userData.targetDistanceCurrentYear");
      Serial.println(userData.targetDistanceCurrentYear);
      Serial.println("------------------");
      //last avtivity
      userData.lastActivityDate = isoToShortDate(doc["data"]["latestRideActivity"]["start_date"].as<String>());
      userData.lastActivityDistance = doc["data"]["latestRideActivity"]["distance"];
      userData.lastActivityDistance = userData.lastActivityDistance/1000;
      userData.lastActivityAVGSpeed = doc["data"]["latestRideActivity"]["formattedAverageSpeed"].as<String>();
      userData.lastActivityMovingTime = doc["data"]["latestRideActivity"]["formattedMovingTime"].as<String>();
      //records
      userData.biggestClimb = doc["data"]["maxRideStats"]["maxElevation"];
      userData.longestDistance = doc["data"]["maxRideStats"]["maxDistance"];
      //updatetime
      userData.lastUpdateFromStrava = doc["data"]["lastUpdatedFormatted"].as<String>();


    //   // display.fillScreen(GxEPD_WHITE);
    //   // display.setCursor(10, 20);
    //   // display.print("Name: ");
    //   // display.println(userData.username);
    //   // display.setCursor(10, 40);
    //   // display.print("year: ");
    //   // display.println(userData.currentYear);
    //   // display.setCursor(10, 60);
    //   // display.print("totalDistance: ");
    //   // display.println(userData.totalDistanceCurrentYear);
    //   // display.update();
    //   Serial.println("Waiting for 2 seconds...");
    //   delay(2000);

       displayStravaTotalRide();
       WiFi.disconnect(true);

    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }
}

String isoToShortDate(const String& isoDate) {
  int month = isoDate.substring(5, 7).toInt();
  int day   = isoDate.substring(8, 10).toInt();

  const char* months[] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
  };

  char buf[16];
  sprintf(buf, "%s %d", months[month - 1], day);

  return String(buf);
}

//页面相关函数
// ================= 页码变量 =================
int currentPage = 0; // 当前页
const int totalPages = 4; // 总页数（根据需要改）

void displayStravaTotalRide() {

 
  //绘制上部分黑底
  display.fillRect(0, 0, 250, 82, GxEPD_BLACK);
  display.fillRoundRect(8, 23, 8, 44, 3, GxEPD_WHITE);
  
  drawStatusBar(true);

  display.setCursor(24, 35);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  Serial.print(userData.currentYear);
  display.print(String(userData.currentYear) + " Ride Distance");

  kilometerText(24,65,userData.totalDistanceCurrentYear,true,"km");
  Serial.println("@@@@@@@@");
  Serial.print(userData.totalDistanceCurrentYear);
  Serial.print(userData.targetDistanceCurrentYear);
  float percent = userData.totalDistanceCurrentYear / userData.targetDistanceCurrentYear * 100;
  Serial.print(percent);
  //进度条
  drawProgressBar(20,95,170,20,percent);

  // 更新显示屏
  display.update();
}
//显示总爬升
void displayStravaBiggestRecord() {
  drawStatusBar(false);

   // 在(10,10)位置绘制
  drawTrophyIcon(8,32);
  drawTrophyIcon(8,83);

  display.fillRoundRect(42, 23, 4, 40, 3, GxEPD_BLACK);
  display.fillRoundRect(42, 74, 4, 40, 3, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);

  display.setCursor(52, 32);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.print("Biggest Climb");
  kilometerText(52,63,userData.biggestClimb,false,"m");

  display.setCursor(52, 81);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.print("Longest Ride");
  kilometerText(52,114,userData.longestDistance,false,"km");


  display.update();
}
//顶部状态栏绘制
void drawStatusBar(bool whiteColor) {

  if(whiteColor) {
    display.setTextColor(GxEPD_WHITE);
  } else {
    display.setTextColor(GxEPD_BLACK);
  }
  display.setFont(&TomThumb);
  
  // 设置字体
  display.setTextSize(1.5);

  // 绘制文本
  display.setCursor(8, 15);
  display.setTextSize(1.8);
  display.print(userData.username);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(userData.lastUpdateFromStrava,0,0,&x1,&y1,&w,&h);
  display.setCursor(250-w-10, 15);
  display.print(userData.lastUpdateFromStrava);

}
//公里数
void kilometerText(int x, int y, double numStr,bool whiteColor,const char* unit) {
  
  if(whiteColor) {
    display.setTextColor(GxEPD_WHITE);
  } else {
    display.setTextColor(GxEPD_BLACK);
  }
  display.setFont(&FreeMonoBold18pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  display.setCursor(x, y);
  display.setTextSize(1);
  display.print(numStr);
  display.getTextBounds(String(numStr,2),x,y,&x1,&y1,&w,&h);
  display.setCursor(x+w+10, y);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.print(unit);
}
// 绘制一个简单的骑行图标
void drawTrophyIcon(int x, int y) {
  display.drawBitmap(
    x, y,
    bitmap_zwlu4g,
    ZWLU4G_BMPWIDTH, 22,   // 宽=26, 高需要自己填，比如22像素
    GxEPD_BLACK
  );
}

void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent)
{
  if (percent > 100) percent = 100;

  // 外框（带圆角矩形）
  display.drawRoundRect(x, y, w, h, h/2, GxEPD_BLACK);

  // 填充进度（根据百分比计算宽度）
  int16_t fillWidth = (w - 4) * percent / 100; // 留点边距
  if (fillWidth > 0) {
    display.fillRoundRect(x + 2, y + 2, fillWidth, h - 4, (h-4)/2, GxEPD_BLACK);
  }

  // 阴影/高光效果（在已填充部分顶部绘制一条白线，让进度条更有层次感）
  if (fillWidth > 4) {
    display.drawLine(x + 3, y + 3, x + 3 + fillWidth - 2, y + 3, GxEPD_WHITE);
  }

  // 显示百分比文本
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  char buf[8];
  sprintf(buf, "%d%%", percent);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(buf, x, y, &tbx, &tby, &tbw, &tbh);
  display.setCursor(x + w + 10, y + h/2 + tbh/2 - 2);
  display.print(buf);
}

void displayStravaLastRide() {
   
  //绘制上部分黑底
  display.fillRect(0, 0, 250, 82, GxEPD_BLACK);
  display.fillRoundRect(8, 23, 8, 44, 3, GxEPD_WHITE);
  
  drawStatusBar(true);

  display.setCursor(24, 35);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.print("Last Ride on " + userData.lastActivityDate);

  kilometerText(24,65,userData.lastActivityDistance,true,"km");

  //下半部分黑色字体
  display.setTextColor(GxEPD_BLACK);

  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(8, 100); 
  display.setTextSize(1);
  display.print("AVG");
  display.setCursor(120, 100); 
  display.print("Moving Time");

  display.setCursor(8, 118); 
  display.setTextSize(1.5);
  display.print(userData.lastActivityAVGSpeed);
  display.setCursor(120, 118); 
  display.print(userData.lastActivityMovingTime);



  // 更新显示屏
  display.update();
}

void drawPage2() {
  display.setCursor(10, 50);
  display.print("Page 2: Goodbye");
}

void outForRide() {
  display.fillScreen(GxEPD_WHITE);
  const size_t len = sizeof(bitmap_waitRide) / sizeof(bitmap_waitRide[0]);
  uint8_t bitmap_inverted[len];

  for (size_t i = 0; i < len; i++) {
    bitmap_inverted[i] = ~bitmap_waitRide[i];
  }
  display.drawBitmap(
    24, 28,
    bitmap_inverted,
    192, 66,   // 宽=26, 高需要自己填，比如22像素
    GxEPD_BLACK
  );
  display.update();

}

void wifiTimerCallback(void* arg) {
  Serial.println("6小时定时器触发，准备连接WiFi");
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.begin();
  }
}


void setup() {
  Serial.setDebugOutput(true);
  pinMode(BUTTON_PIN, INPUT);


  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  Serial.print("Flash size (MB): ");
  Serial.println(ESP.getFlashChipSize() / (1024 * 1024));

  display.init();  // enable diagnostic output on Serial

  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(32, 32);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.print("Connecting Wifi...");

  display.update();

  // WiFi 配置
  const char* ssid = "";
  const char* password = "";

  WiFi.persistent(true);

  WiFi.begin(); 
  //WiFi.begin() // no SSID/PWD - get it from the Provisioning APP or from NVS (last successful connection)
  WiFi.onEvent(SysProvEvent);
  Serial.print("Try Connecting with saved WiFi");
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if(WiFi.status() != WL_CONNECTED ) {
    // BLE Provisioning using the ESP SoftAP Prov works fine for any BLE SoC, including ESP32, ESP32S3 and ESP32C3.
    #if (defined(CONFIG_BLUEDROID_ENABLED) || defined(CONFIG_NIMBLE_ENABLED)) && __has_include("esp_bt.h") && !defined(USE_SOFT_AP)
      Serial.println("no saved wifi");
      Serial.println("Begin Provisioning using BLE");
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(40,40);
      display.println("Provisioning using BLE");
      display.update();
      // Sample uuid that user can pass during provisioning using BLE
      uint8_t uuid[16] = { 0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf, 0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };
      WiFiProv.beginProvision(
        NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BLE, NETWORK_PROV_SECURITY_1, pop, service_name, service_key, uuid, reset_provisioned);
      log_d("ble qr");
      WiFiProv.printQR(service_name, pop, "ble");
    #else
      Serial.println("Begin Provisioning using Soft AP");
      WiFiProv.beginProvision(NETWORK_PROV_SCHEME_SOFTAP, NETWORK_PROV_SCHEME_HANDLER_NONE, NETWORK_PROV_SECURITY_1, pop, service_name, service_key);
      log_d("wifi qr");
      WiFiProv.printQR(service_name, pop, "softap");
    #endif
  }else {
    // ✅ 成功连接
    Serial.println("\nConnected using saved WiFi config!");
    Serial.println(WiFi.localIP());
    getUserInfo();
    
  }
  esp_timer_create_args_t timer_args = {
    .callback = &wifiTimerCallback,
    .name = "wifi_timer"
  };
  esp_timer_handle_t wifi_timer;
  esp_timer_create(&timer_args, &wifi_timer);
  esp_timer_start_periodic(wifi_timer, 6 * 60 * 60 * 1000000); // 6小时，单位微秒

  Serial.println("setup done");
  

}


void loop() {
  int buttonState = digitalRead(BUTTON_PIN);

  // 检测按下事件（从 HIGH 变成 LOW）
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Button Pressed!");
    
    currentPage++;
    if (currentPage >= totalPages) {
      currentPage = 0; // 循环翻页
    }

    // 清屏 & 显示对应页面
    display.fillScreen(GxEPD_WHITE);
  
      switch (currentPage) {
        case 0: displayStravaTotalRide(); break;
        case 1: displayStravaBiggestRecord();  break;
        case 2: displayStravaLastRide(); break;
        case 3: outForRide(); break;
      }
    
    // 刷新屏幕
    display.update();
  }

  lastButtonState = buttonState;
  delay(10);  // 简单防抖

}

