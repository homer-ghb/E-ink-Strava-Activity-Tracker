# Strava 客户端显示屏

一个基于 ESP32 和电子墨水屏的 Strava 运动数据显示器，通过 WiFi 连接获取 Strava 运动数据并显示在电子墨水屏上。

## 项目介绍

本项目是一个智能 Strava 数据显示器，具有以下功能：

- 📊 **年度骑行数据统计**：显示当前年度总骑行距离和完成进度
- 🏆 **个人最佳记录**：展示最大爬升高度和最长骑行距离
- 🚴 **最近一次骑行**：显示最近一次骑行的详细信息
- 🎨 **精美UI设计**：使用电子墨水屏提供低功耗的清晰显示
- 📱 **智能WiFi配置**：支持BLE和SoftAP两种WiFi配网方式
- 🔘 **按键交互**：通过按键切换不同显示页面

## 硬件选型

### 主要组件

| 组件 | 型号/规格 | 说明 |
|------|-----------|------|
| 主控板 | ESP32 | 支持WiFi和BLE的微控制器 |
| 显示屏 | GxDEPG0213BN | 2.13英寸电子墨水屏，分辨率250x122 |
| 按键 | 轻触开关 | 用于页面切换 |

### 引脚连接

| ESP32引脚 | 连接组件 | 说明 |
|-----------|----------|------|
| GPIO23 | MOSI | SPI数据线 |
| GPIO18 | CLK | SPI时钟线 |
| GPIO5 | CS (ELINK_SS) | 显示屏片选 |
| GPIO4 | BUSY | 显示屏忙信号 |
| GPIO16 | RST (ELINK_RESET) | 显示屏复位 |
| GPIO17 | DC | 显示屏数据/命令选择 |
| GPIO39 | BUTTON_PIN | 按键输入 |

### 硬件连接图

```
ESP32         电子墨水屏
GPIO23  -----> MOSI
GPIO18  -----> CLK  
GPIO5   -----> CS
GPIO4   -----> BUSY
GPIO16  -----> RST
GPIO17  -----> DC
3.3V    -----> VCC
GND     -----> GND

ESP32         按键
GPIO39  -----> 按键一端
GND     -----> 按键另一端
```

## 软件环境配置

### Arduino IDE 配置

1. **安装 Arduino IDE**
   - 下载并安装 Arduino IDE (推荐版本 1.8.x 或 2.x)

2. **添加 ESP32 开发板支持**
   - 打开 Arduino IDE
   - 文件 → 首选项
   - 在"附加开发板管理器网址"中添加：
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - 工具 → 开发板 → 开发板管理器
   - 搜索"ESP32"并安装"ESP32 by Espressif Systems"

3. **安装必需的库**
   
   通过库管理器安装以下库：
   - `GxEPD` - 电子墨水屏驱动库
   - `ArduinoJson` - JSON解析库
   - `WiFiProv` - WiFi配网库 (通常随ESP32核心库一起安装)

### 开发板设置

在 Arduino IDE 中进行以下设置：

- **开发板**：选择 "ESP32 Dev Module"
- **端口**：选择正确的COM端口
- **Flash Size**：设置为 "4MB (32Mb)"
- **Partition Scheme**：设置为 "Default 4MB with spiffs"
- **Upload Speed**：设置为 "921600"

## 烧录程序

### 1. 准备工作

1. 确保硬件连接正确
2. 将ESP32通过USB线连接到电脑
3. 打开 Arduino IDE

### 2. 配置项目

1. 打开 `stravaClientWithUI.ino` 文件
2. 修改API地址（如果需要）：
   ```cpp
   const char* serverName = "https://api.uxengineer.top/api/device/a2346ba083c547f7/dashboard-data";
   ```
3. 检查引脚配置是否与硬件匹配

### 3. 编译和上传

1. 点击"验证"按钮检查代码是否有错误
2. 如果编译成功，点击"上传"按钮
3. 等待上传完成

### 4. 上传注意事项

- 如果上传失败，尝试以下方法：
  - 按住ESP32的BOOT按钮，然后按一下RESET按钮，再松开BOOT按钮
  - 降低上传波特率到115200
  - 检查USB线是否支持数据传输

## WiFi 连接和配对

### 方法一：BLE配网（推荐）

1. **启动配网模式**
   - 设备启动后如果没有保存的WiFi信息，会自动进入配网模式
   - 屏幕会显示"Provisioning using BLE"

2. **使用手机APP配网**
   - 下载"ESP BLE Provisioning"APP
   - 扫描二维码或手动输入设备信息：
     - Service Name: `PROV_123`
     - POP: `abcd1234`
   - 选择你的WiFi网络并输入密码

3. **完成配网**
   - 配网成功后设备会自动连接到WiFi
   - 开始获取Strava数据并显示

### 方法二：SoftAP配网

1. **启用SoftAP模式**
   - 在代码中取消注释：`#define USE_SOFT_AP`
   - 重新编译上传

2. **连接设备热点**
   - 手机WiFi中找到设备创建的热点
   - 连接后会自动打开配网页面
   - 输入你的WiFi信息

### 方法三：代码中直接配置

在 `setup()` 函数中修改WiFi信息：

```cpp
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";
```

## 使用说明

### 按键操作

- **短按按键**：切换到下一个显示页面
- **页面循环**：总共4个页面，循环显示

### 显示页面

1. **页面1 - 年度骑行统计**
   - 显示当前年度总骑行距离
   - 显示年度目标的完成进度条
   - 显示用户名和最后更新时间

2. **页面2 - 个人最佳记录**
   - 最大爬升高度
   - 最长骑行距离
   - 奖杯图标装饰

3. **页面3 - 最近一次骑行**
   - 骑行日期
   - 骑行距离
   - 平均速度
   - 移动时间

4. **页面4 - 等待骑行**
   - 显示骑行图标
   - 提示用户去骑行

## 故障排除

### 常见问题

1. **屏幕显示花屏**
   - 尝试注释/取消注释不同的屏幕驱动头文件
   - 检查屏幕型号是否匹配

2. **WiFi连接失败**
   - 检查WiFi名称和密码是否正确
   - 确保WiFi信号强度足够
   - 尝试重新配网

3. **无法获取数据**
   - 检查API地址是否正确
   - 确认网络连接正常
   - 查看串口输出的错误信息

4. **按键无响应**
   - 检查按键连接是否正确
   - 确认按键没有短路或断路

### 串口调试

- 打开串口监视器，波特率设置为115200
- 观察启动信息和错误日志
- 根据错误信息进行相应处理

## 项目结构

```
stravaClientWithUI/
├── stravaClientWithUI.ino    # 主程序文件
├── trophyBitmap.h            # 奖杯图标位图数据
├── waitforride.h             # 骑行等待图标位图数据
└── README.md                 # 项目说明文档
```

## 开发信息

- **开发环境**：Arduino IDE
- **目标平台**：ESP32
- **主要依赖**：GxEPD库、ArduinoJson库、WiFiProv库
- **编程语言**：C++

## 许可证

本项目仅供学习和个人使用。

## 更新日志

- v1.0.0 - 初始版本，支持基本的Strava数据显示功能
- 支持多种屏幕型号
- 实现WiFi配网功能
- 添加按键交互

---

如有问题或建议，欢迎提交Issue或Pull Request。
