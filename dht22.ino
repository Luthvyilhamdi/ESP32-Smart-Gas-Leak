#define BLYNK_TEMPLATE_ID "TMPL6CdHiAEvU"
#define BLYNK_TEMPLATE_NAME "SMART GAS"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Tentukan Pin Untuk Perangkat
#define sensorMQ2 34
#define sensorFlame 35
#define sensorPIR 26
#define sensorDHT 27
#define relayFan1 14
#define relayFan2 17
#define relayPump 16
#define buzzer 2
#define ledRed 25
#define ledGreen 33
#define ledYellow 32
#define ledBlue 4

// Inisiasikan Wifi Untuk Blynk
char auth[] = "MIMnpH0-B-L0EqAdrPkRmZbM9f7UbgJj";
char ssid[] = "BismillahBerhasil";
char pass[] = "terusberusaha";

// Inisialisasi layar LCD dan sensor DHT
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(sensorDHT, DHT22);

// Prototipe fungsi
void buzz(int duration, int repeat);
void handleBuzzer(bool gasDetected, bool flameDetected, bool motionDetected, bool highTempDetected);
void sendNotification(const char* message);
void handleNotifications(bool gasDetected, bool flameDetected, bool motionDetected, bool highTempDetected);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht.begin();

  // Konfigurasikan pin
  pinMode(sensorMQ2, INPUT);
  pinMode(sensorFlame, INPUT);
  pinMode(sensorPIR, INPUT);
  pinMode(relayFan1, OUTPUT);
  pinMode(relayFan2, OUTPUT);
  pinMode(relayPump, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("System Loading...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready");

  // Hubungkan ke Wi-Fi dan Blynk
  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();

  // Baca level gas dari sensor MQ-2
  int gasValue = analogRead(sensorMQ2);
  float gasVoltage = (gasValue / 4095.0) * 5.0;
  float ppm = (gasVoltage / 5.0) * 10000.0;
  bool gasDetected = ppm >= 300.0;

  // Perbarui Blynk untuk sensor gas
  Blynk.virtualWrite(V0, ppm);
  Blynk.virtualWrite(V1, gasDetected ? "Bahaya" : "Aman");

  // Baca sensor api
  bool flameDetected = !digitalRead(sensorFlame);
  Blynk.virtualWrite(V2, flameDetected ? "Terdeteksi" : "Tidak Terdeteksi");

  // Baca sensor PIR
  bool motionDetected = digitalRead(sensorPIR);
  Blynk.virtualWrite(V3, motionDetected ? "Gerakan Terdeteksi" : "Tidak Ada Gerakan");

  // Baca suhu dan kelembaban dari sensor DHT22r
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  bool highTempDetected = temperature > 40.0;

  // Perbarui Blynk untuk suhu dan kelembapan
  if (!isnan(temperature)) Blynk.virtualWrite(V4, temperature);
  if (!isnan(humidity)) Blynk.virtualWrite(V5, humidity);

  // Menampilkan level dan kondisi gas pada LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(ppm, 1);

  lcd.setCursor(0, 1);
  if (gasDetected) {
    lcd.print("Kondisi: Bahaya");
    digitalWrite(relayFan1, HIGH);
    digitalWrite(relayFan2, HIGH);
    digitalWrite(ledRed, HIGH);
  } else {
    lcd.print("Kondisi: Aman");
    digitalWrite(relayFan1, LOW);
    digitalWrite(relayFan2, LOW);
    digitalWrite(ledRed, LOW);
  }

  // Tangani deteksi api
  if (flameDetected) {
    digitalWrite(relayPump, HIGH);
    digitalWrite(ledGreen, HIGH);
    Serial.println("Api Terdeteksi! Water pump ON.");
  } else {
    digitalWrite(relayPump, LOW);
    digitalWrite(ledGreen, LOW);
  }

  // Menangani deteksi gerakan PIR
  if (motionDetected) {
    digitalWrite(ledYellow, HIGH);
    Serial.println("Gerakan Terdeteksi!");
  } else {
    digitalWrite(ledYellow, LOW);
  }

  // Tangani deteksi suhu
  if (!isnan(temperature)) {
    if (highTempDetected) {
      digitalWrite(ledBlue, HIGH);
      Serial.println("Suhu Tinggi Terdeteksi!");
    } else {
      digitalWrite(ledBlue, LOW);
    }
  }

  // Print gas data to Serial Monitor
  Serial.println("============================");
  Serial.print("ADC Value: ");
  Serial.println(gasValue);

  Serial.print("Voltage: ");
  Serial.print(gasVoltage, 2);
  Serial.println(" V");

  Serial.print("Gas PPM: ");
  Serial.print(ppm, 1);
  Serial.println(" PPM");

  // Print temperature and humidity data
  if (!isnan(temperature) && !isnan(humidity)) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Gagal Membaca Data Dari Sensor DHT!");
  }

  Serial.println("============================");

  // Menangani bel dan notifikasi
  handleBuzzer(gasDetected, flameDetected, motionDetected, highTempDetected);
  handleNotifications(gasDetected, flameDetected, motionDetected, highTempDetected);

  delay(1000);
}

// Berfungsi untuk menghasilkan suara buzzer
void buzz(int duration, int repeat) {
  for (int i = 0; i < repeat; i++) {
    digitalWrite(buzzer, HIGH);
    delay(duration);
    digitalWrite(buzzer, LOW);
    delay(100);
  }
}

// Berfungsi untuk menangani suara buzzer
void handleBuzzer(bool gasDetected, bool flameDetected, bool motionDetected, bool highTempDetected) {
  if (gasDetected) buzz(1000, 1);
  else if (flameDetected) buzz(200, 5);
  else if (motionDetected) buzz(300, 2);
  else if (highTempDetected) buzz(400, 3);
  else digitalWrite(buzzer, LOW);
}

// Berfungsi untuk mengirim notifikasi
void sendNotification(const char* message) {
  Blynk.logEvent("alert", message);
}

// Berfungsi untuk menangani notifikasi
void handleNotifications(bool gasDetected, bool flameDetected, bool motionDetected, bool highTempDetected) {
  if (gasDetected) sendNotification("Bahaya Gas Terdeteksi! Segera Periksa!");
  if (flameDetected) sendNotification("Bahaya Api Terdeteksi! Segera Ambil Tindakan!");
  if (motionDetected) sendNotification("Gerakan Terdeteksi di Area! Periksa Keamanan!");
  if (highTempDetected) sendNotification("Suhu Tinggi Terdeteksi! Segera Periksa!");
}
