#define BLYNK_TEMPLATE_ID "Template ID"
#define BLYNK_TEMPLATE_NAME "Project IoT"
#define BLYNK_AUTH_TOKEN "auth-token"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#define DHTPIN 15         
#define DHTTYPE DHT22
#define RELAY_FAN_PIN 27  
#define BUZZER_PIN 25   

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "mier_iot";        
char pass[] = "mier1234";   

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

int manualFan = 0;  
int manualBuzzer = 0;  
bool isNotified = false;

BLYNK_WRITE(V2) {
  manualFan = param.asInt();
  Serial.print("[Blynk] Kontrol Manual Kipas: ");
  Serial.println(manualFan ? "ON" : "OFF");
}

BLYNK_WRITE(V3) {
  manualBuzzer = param.asInt();
  Serial.print("[Blynk] Kontrol Manual Buzzer: ");
  Serial.println(manualBuzzer ? "ON" : "OFF");
}

void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Gagal membaca sensor DHT22!");
    return;
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  int finalFanState = LOW;
  int finalBuzzerState = LOW;

  Serial.print("Waktu: "); 
  Serial.print(millis()); 
  Serial.print(" ms | Suhu: "); 
  Serial.print(t); 
  Serial.print("°C | Status: ");

  if (t >= 45.0) {
    finalFanState = LOW;      
    finalBuzzerState = HIGH;  
    
    Serial.println("BAHAYA (Alarm Indikasi Kebakaran Menyala)"); 
    
    if (!isNotified) {
      Blynk.logEvent("fire_alert", "BAHAYA! Suhu kritis: " + String(t) + "°C");
      isNotified = true;
    }
  } 
  else if (t >= 35.0) {
    
    finalFanState = HIGH;     
    finalBuzzerState = LOW;   
    isNotified = false;       
    
    Serial.println("PANAS (Kipas Menyala)"); 
  } 
  else {
    
    finalFanState = LOW;
    finalBuzzerState = LOW;
    isNotified = false;
    
    Serial.println("NORMAL"); 
  }
  
  if (manualFan == 1) finalFanState = HIGH;
  if (manualBuzzer == 1) finalBuzzerState = HIGH;
  
  digitalWrite(RELAY_FAN_PIN, finalFanState);
  digitalWrite(BUZZER_PIN, finalBuzzerState);
  
  Blynk.virtualWrite(V2, finalFanState);
  Blynk.virtualWrite(V3, finalBuzzerState);
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("\n=== SISTEM IoT DIMULAI ===");

  pinMode(RELAY_FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
   
  digitalWrite(RELAY_FAN_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  
  Serial.println("Menghubungkan ke WiFi & Blynk...");
  Blynk.begin(auth, ssid, pass);
  
  Serial.println("Sistem Online!");
 
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}