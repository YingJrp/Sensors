#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <HT_SSD1306Wire.h> // ใช้ไลบรารีสำหรับ OLED จาก Heltec
#include <DHT.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>

// ตั้งค่าหน้าจอ OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 16
static SSD1306Wire display_screen(0x3c,
                                  500000,
                                  SDA_OLED,
                                  SCL_OLED,
                                  GEOMETRY_128_64,
                                  RST_OLED);

// ตั้งค่าเซ็นเซอร์ DHT22
#define DHTPIN 12 // พิน GPIO ที่เชื่อมกับขา Data ของ DHT22
#define DHTTYPE DHT22 // ประเภทเซ็นเซอร์ DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // เริ่มต้นการใช้งาน Serial Monitor
  Serial.begin(9600);

  // เริ่มต้นการทำงานของเซ็นเซอร์ DHT
  dht.begin();

  // เริ่มต้นการทำงานของหน้าจอ OLED
  display_screen.init();
  display_screen.clear();
  display_screen.setFont(ArialMT_Plain_10); // ตั้งค่าฟอนต์
}

void loop() {
  // อ่านค่าอุณหภูมิในหน่วยองศาเซลเซียสและค่า humidity จาก DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // ตรวจสอบว่าการอ่านค่าล้มเหลวหรือไม่
  // if (isnan(temperature) || isnan(humidity)) {
  //   Serial.println(F("การอ่านค่าจากเซ็นเซอร์ DHT ล้มเหลว!"));
  //   return;
  // }

  // แสดงผลทาง Serial Monitor
  Serial.print(F("อุณหภูมิ: "));
  Serial.print(temperature);
  Serial.print(F("°C  "));
  Serial.print(F("ความชื้น: "));
  Serial.print(humidity);
  Serial.println(F("%"));

  // แสดงผลบนหน้าจอ OLED
  display_screen.clear(); // ล้างหน้าจอ
  display_screen.drawString(0, 10, "Temp: " + String(temperature) + " C");
  display_screen.drawString(0, 30, "Humidity: " + String(humidity) + " %");
  display_screen.display();

  // รอ 2 วินาทีระหว่างการอ่านค่า
  delay(2000);
}