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

// ตั้งค่าเซ็นเซอร์ DHT22
#define DHTPIN 32 // พิน GPIO ที่เชื่อมกับขา Data ของ DHT22
#define DHTTYPE DHT22 // ประเภทเซ็นเซอร์ DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MQ7_PIN 13  // ขา ADC ที่เชื่อมต่อกับ MQ-7
MQ135 mq135_sensor(33); // ใช้ GPIO36 (A0) สำหรับเซ็นเซอร์ MQ-135

//float R0 = 10.0; // ตั้งค่าเริ่มต้น R₀
float R0;

// ตั้งเวลาในการอุ่นเครื่องและตรวจจับ
const int heatingTime = 60000; // 60 วินาทีในการอุ่นเครื่อง
const int sensingTime = 90000; // 90 วินาทีสำหรับการวัด

void setup() {
  // เริ่มต้นการใช้งาน Serial Monitor
  Heltec.begin(false /*จอ OLED*/, false /*ลอรา*/, true /*ซีเรียล*/, true, 923.3E6);
  Serial.begin(9600);

  // อุ่นเครื่องเซ็นเซอร์ MQ-135
  delay(3000); // 30 วินาที

  // เริ่มต้นการทำงานของเซ็นเซอร์ DHT
  dht.begin();

  // คำนวณค่า R₀ ในสภาพอากาศบริสุทธิ์
  float rs_in_clean_air = mq135_sensor.getResistance();
  R0 = rs_in_clean_air / pow(400.0 / 116.6020682, -0.42); // คาลิเบรตค่า R0
  Serial.print("Rs : ");
  Serial.println(rs_in_clean_air);
  Serial.print("Calibrated R0 : ");
  Serial.println(R0);

  // เริ่มต้นการทำงานของหน้าจอ OLED
  display_screen.init();
  display_screen.clear();
  display_screen.setFont(ArialMT_Plain_10); // ตั้งค่าฟอนต์
  // display_screen.drawString(0, 0, "Temp Humidity CO CO2 Monitoring");
  display_screen.display();
  delay(5000);  

  pinMode(MQ7_PIN, INPUT);  // ตั้งค่าขาเป็น input
}

float readCOppm() {
  int sensorValue = analogRead(MQ7_PIN);  // อ่านค่าจากเซ็นเซอร์
  float voltage = (sensorValue / 4095.0) * 5;  // แปลงค่าเป็นแรงดันไฟฟ้า (ESP32 มี ADC 12 บิต, แรงดัน 5V)
  return voltage;  // คำนวณค่า PPM
}

void loop() {
  // อ่านค่าอุณหภูมิและความชื้นจาก DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // อุ่นเครื่อง MQ-7
  digitalWrite(MQ7_PIN, HIGH);  // เปิดฮีตเตอร์
  // delay(heatingTime);           // อุ่นเครื่อง
  digitalWrite(MQ7_PIN, LOW);   // ปิดฮีตเตอร์

  // อ่านค่า CO จาก MQ-7
  float co_ppm = readCOppm();

  // อ่านค่า CO2 จาก MQ-135
  float rs = mq135_sensor.getResistance();
  float ppm = 116.6020682 * pow((rs / R0), -0.42); // คำนวณค่า PPM

  display_screen.drawString(0, 0, "Temp Humidity CO CO2 Monitoring");

  // แสดงผลทาง Serial Monitor
  
  Serial.print(F("อุณหภูมิ : "));
  Serial.print(temperature);
  Serial.print(F("°C  ความชื้น : "));
  Serial.print(humidity);
  Serial.println(F("%"));

  // แสดงผลบนหน้าจอ OLED
  display_screen.clear(); // ล้างหน้าจอ
  display_screen.drawString(0, 10, "Temp : " + String(temperature) + " C");
  display_screen.drawString(0, 20, "Humidity : " + String(humidity) + " %");

  Serial.print("CO2 : ");
  Serial.print(ppm);
  Serial.println(" PPM");

  display_screen.drawString(0, 30, "CO2 : " + String(ppm) + " PPM");

  Serial.print("CO : ");
  Serial.print(co_ppm);
  Serial.println(" PPM");

  display_screen.drawString(0, 40, "CO : " + String(co_ppm) + " PPM");
  
  // เงื่อนไขในการแสดงข้อความตามค่า CO
  if (co_ppm < 30 && ppm < 500) {
    //display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 50, "Safe");
  } else if ((co_ppm >= 30 && co_ppm <= 100) && (ppm >= 500 && ppm <= 5000) && (temperature>=32) && (humidity<=78)) {
    //display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 50, "Warning: Fire starting!");
  } else if ((co_ppm > 100 && co_ppm < 800) && (ppm >= 5000 && ppm <= 40000) && (temperature>33) && (62>=humidity<=72)) {
    //display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 50, "Danger: Full fire!");
  } else {
    //display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
    display_screen.drawString(0, 50, "Dangerous to Human and Life!");
  }

  display_screen.display(); // อัปเดตหน้าจอ

  // รอ 2 วินาทีระหว่างการอ่านค่า
  delay(2000);
}