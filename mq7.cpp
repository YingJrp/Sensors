#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "heltec.h"  // ไลบรารีสำหรับหน้าจอ Heltec
#include <HT_SSD1306Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>

#define MQ7_PIN 13  // ขา ADC ที่เชื่อมต่อกับ MQ-7
static SSD1306Wire display_screen(0x3c,
                                  500000,
                                  SDA_OLED,
                                  SCL_OLED,
                                  GEOMETRY_128_64,
                                  RST_OLED);

// ฟังก์ชันในการอ่านค่า CO จาก MQ-7
float readCOppm() {
  int sensorValue = analogRead(MQ7_PIN);  // อ่านค่าจากเซ็นเซอร์
  float voltage = (sensorValue / 4095.0) * 3.3;  // แปลงค่าเป็นแรงดันไฟฟ้า (ESP32 มี ADC 12 บิต, แรงดัน 3.3V)
  return voltage;
}
// --------------------------------------------------------------------
  // // แปลงค่าแรงดันเป็นค่า ppm (ตัวอย่างสูตร, ต้องการคาลิเบรทเพิ่มเติม)
  // float ppm = pow(10, ((voltage - 0.1) / 0.05));
  
  // // return ppm;
// --------------------------------------------------------------------

void setup() {
  // เริ่มต้นหน้าจอ OLED
  Heltec.begin(false /*จอ OLED*/, false /*ลอรา*/, true /*ซีเรียล*/, true, 923.3E6);
  Serial.begin(9600);
  
  // ล้างหน้าจอ
  display_screen.init();
  display_screen.clear();
  display_screen.drawString(0, 0, "CO Monitoring");
  display_screen.display();
  delay(5000);
  pinMode(MQ7_PIN, INPUT);  // ตั้งค่าขาเป็น input
}

void loop() {
  float co_ppm = readCOppm();  // อ่านค่า CO
  Serial.println(String(co_ppm));
  display_screen.clear();  // เคลียร์หน้าจอ

  // เงื่อนไขในการแสดงข้อความตามค่า CO
  if (co_ppm >= 30 && co_ppm < 100) {
    display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 20, "Warning: Fire starting!");
  } else if (co_ppm >= 100 && co_ppm <= 800) {
    display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 20, "Danger: Full fire!");
  } else if (co_ppm > 800 ) {
    display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 20, "Dangerous to Human and Life!");
  } else {
    display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 20, "Safe.");
  }

  display_screen.display();  // อัปเดตหน้าจอ

  delay(2000);  // หน่วงเวลา 1 วินาที
}