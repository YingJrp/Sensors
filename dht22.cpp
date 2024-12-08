#include <Arduino.h>
#include <heltec.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <HT_SSD1306Wire.h>
#include <MQ135.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>


#define MQ7_PIN 25  // ขา ADC ที่เชื่อมต่อกับ MQ-7
MQ135 mq135_sensor(13); // เซ็นเซอร์ MQ-135 ต่อเข้ากับ GPIO 25

// ตั้งค่าเซ็นเซอร์ DHT22
#define DHTPIN 33 // พิน GPIO ที่เชื่อมกับขา Data ของ DHT22
#define DHTTYPE DHT22 // ประเภทเซ็นเซอร์ DHT22
DHT dht(DHTPIN, DHTTYPE);

// ตั้งค่าหน้าจอ OLED
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET    16
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
  
  // แปลงค่าแรงดันเป็นค่า ppm (ตัวอย่างสูตร, ต้องการคาลิเบรทเพิ่มเติม)
  float ppm = pow(10, ((voltage - 0.1) / 0.05));

  return ppm;
}

void setup() {
  Heltec.begin(false /*จอ OLED*/, false /*ลอรา*/, true /*ซีเรียล*/, true, 923.3E6);
  Serial.begin(9600);
  dht.begin();
  display_screen.setFont(ArialMT_Plain_10); // ตั้งค่าฟอนต์

  // ล้างหน้าจอ
  display_screen.init();
  display_screen.clear();
  display_screen.drawString(0, 0, "CO Monitoring");
  display_screen.display();
  delay(5000);
  pinMode(MQ7_PIN, INPUT);  // ตั้งค่าขาเป็น input
}

void loop() {
  //----------------------------------------Sensor ทำการอ่านค่า CO---------------------------------------------------
  float co_ppm = readCOppm();  // อ่านค่า CO
  Serial.print("Concentration of CO : ");
  Serial.print(String(co_ppm));
  Serial.println(" ppm");
  display_screen.clear();  // เคลียร์หน้าจอ

//   // เงื่อนไขในการแสดงข้อความตามค่า CO
//   if (co_ppm >= 30 && co_ppm < 100) {
//     display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
//     display_screen.drawString(0, 20, "Warning: Fire starting!");
//   } else if (co_ppm >= 100 && co_ppm <= 800) {
//     display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
//     display_screen.drawString(0, 20, "Danger: Full fire!");
//   } else if (co_ppm > 800 ) {
//     display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
//     display_screen.drawString(0, 20, "Dangerous to Human and Life!");
//   } else {
//     display_screen.drawString(0, 0, "CO Level: " + String(co_ppm) + " ppm");
//     display_screen.drawString(0, 20, "Safe.");
//   }

  display_screen.display();  // อัปเดตหน้าจอ

  delay(2000);  // หน่วงเวลา 2 วินาที

  //----------------------------------------Sensor ทำการอ่านค่า CO2---------------------------------------------------

  float ppm = mq135_sensor.getPPM(); // อ่านค่า CO2 ในหน่วย ppm
  // กำหนดข้อความตามค่า CO2
//   String message;
//   if (ppm < 500) {
//     message = "Safe";
//   } else if (ppm >= 500 && ppm < 5000) {
//     message = "Warning: Fire starting!";
//   } else if (ppm >= 5000 && ppm < 40000) {
//     message = "Danger: Full fire!";
//   } else {
//     message = "Dangerous to Human and Life!";
//   }

  // แสดงผลใน Serial Monitor
  Serial.print("Concentration of CO2 : ");
  Serial.print(ppm);
  Serial.println(" ppm ");
  //Serial.println(message);

  // แสดงผลบนหน้าจอ OLED
  display_screen.clear();
  display_screen.drawString(0, 0, "Concentration of CO2 : ");
  display_screen.drawString(0, 10, String(ppm) + " ppm");
  //display_screen.drawString(0, 20, message);
  display_screen.display();
  
  delay(2000); // อัปเดตทุก ๆ 2 วินาที

  //----------------------------------------Sensor ทำการอ่านค่า Humidity && Temp---------------------------------------------------
    // อ่านค่าอุณหภูมิในหน่วยองศาเซลเซียสและค่า humidity จาก DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // ตรวจสอบว่าการอ่านค่าล้มเหลวหรือไม่
  if (isnan(temperature) || isnan(humidity)) { //เอาไว้เช็คว่าเป็นค่าว่างรึป่าว
    Serial.println(F("การอ่านค่าจากเซ็นเซอร์ DHT ล้มเหลว!"));
    return;
  }

  // แสดงผลทาง Serial Monitor
  Serial.print(F("อุณหภูมิ : "));
  Serial.print(temperature);
  Serial.print(F("°C  "));
  Serial.print(F("ความชื้น : "));
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
