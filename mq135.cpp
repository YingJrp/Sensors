// MQ-135 CARBON DIOXIDE
#include <Arduino.h>
#include <HT_SSD1306Wire.h> // ใช้ไลบรารีสำหรับ OLED จาก Heltec
#include <Wire.h>
#include <SPI.h>
#include <heltec.h>  // ไลบรารีสำหรับหน้าจอ Heltec
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MQ135.h>

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

#define MQ135_PIN 33  // กำหนดพินอะนาล็อกที่ใช้ (เช่นพิน 2)

// RL ดูจากหลังบอร์ด
int MQ_RESISTOR = 1000;

// RO ที่คำนวณได้
long RO = 1090;

//min และ max Rs/Ro
float MINRSRO = 1.12;
float MAXRSRO = 2.30;

// a และ b คำนวณได้จากกราฟ
float a = 4.696;
float b = 0.31;
 
void setup() {
  Serial.begin(9600);
  // Pre heat 20 วินาที
  Serial.println("Pre-heat sensor 20 seconds");
  delay(20000);
  Serial.println("Sensor ready start reading");
}
 
void loop() {
// อ่านค่า ADC และแปลงเป็นค่า Rs พร้อมแสดงค่าทาง Serial Monitor
  int ADCRAW = analogRead(MQ135_PIN);
  float RS = ((1024.0*MQ_RESISTOR)/ADCRAW) - MQ_RESISTOR;
  Serial.print("Rs: ");
  Serial.println(RS);
  
// คำนวณ Rs/Ro พร้อมแสดงค่าทาง Serial Monitor
  float RSRO = RS/RO;
  Serial.print("Rs/Ro: ");
  Serial.println(RSRO);
  
// ถ้า Rs/Ro อยู่ในช่วงที่วัดได้ ให้คำนวณและแสดงค่า ppm 
// ถ้าค่าที่ได้น้อยหือมากเกินไปให้แสดงข้อความว่าเกินช่วงที่เซ็นเซอร์จะวัดได้
  //if(RSRO < MAXRSRO && RSRO > MINRSRO) {
   float ppm = a*pow(RS/RO, b);
   Serial.print("CO2 : ");
   Serial.print(ppm);
   Serial.println(" ppm");
  } 
  //else {
   //Serial.println("Out of range.");
  //}
  //delay(5000);
//}