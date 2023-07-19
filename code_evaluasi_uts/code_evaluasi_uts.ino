// Wifi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
// Buat object Wifi
#include <WiFiClient.h>

WiFiClient wiFiClient;

ESP8266WiFiMulti WiFiMulti;
// Buat object http
HTTPClient http;
#define USE_SERIAL Serial

//send data
String urlSaveData = "http://192.168.114.50/praktek4-ultrasonik-server-lokal/data/saveData?jarak=";
String respon = "";

//wifi
#define WIFI_SSID "Bas"
#define WIFI_PASSWORD "12345678"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Sensor Ultrasonik
#define echoPin D7
#define trigPin D6

int newJarak;

//sensor flame
#define FLAME_PIN D5
int bacasensor = 0;  
String flameKet;

//relay

#define RELAY_PIN D8
#define RELAY2_PIN D9

#define relay_off HIGH
#define relay_on LOW

String statusRelay, statusRelayDua;

void setup() {
  Serial.begin(115200);

  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(false);

  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] Tunggu %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  } 

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  for (int u = 1; u <= 5; u++)
  {
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      USE_SERIAL.println("Terhubung ke wifi");
      USE_SERIAL.flush();
      delay(1000);
    }
    else
    {
      Serial.println("Wifi belum terhubung");
      delay(1000);
    }
  }
  
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, relay_off);
  digitalWrite(RELAY2_PIN, relay_off);
}

void loop() {
  jarak();
  flame();

  kirim_database();

  if (statusRelay.toInt() == 1) {
    digitalWrite(RELAY_PIN, relay_on);
    Serial.println("Relay 1 ON");
  } else {
    digitalWrite(RELAY_PIN, relay_off);
    Serial.println("Relay 1 OFF");
  }

   if (statusRelayDua.toInt() == 1) {
    digitalWrite(RELAY2_PIN, relay_on);
    Serial.println("Relay 2 ON");
  } else {
    digitalWrite(RELAY2_PIN, relay_off);
    Serial.println("Relay 2 OFF");
  }
  
  delay(1000);
}

void flame(){
  //flame
  bacasensor = digitalRead(FLAME_PIN);
   //  flame
  Serial.println("--------- SENSOR API -----");
  if (bacasensor == LOW) {
    // turn LED on:
    digitalWrite(RELAY2_PIN, LOW);
    Serial.println("Terdeteksi Panas Api");
    Serial.println("LED Menyala");
    flameKet = "True";
  } else {
    // turn LED off:
    digitalWrite(RELAY2_PIN, HIGH);
    Serial.println("Tidak Terdeteksi Panas Api");
    Serial.println("LED Tidak Menyala");
    flameKet = "False";
  }
  
  Serial.println();
}

void jarak() {
  int durasi, jarak, pos=0;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  durasi = pulseIn(echoPin, HIGH);
  jarak = (durasi / 2) / 29.1;
//
  if (jarak < 0) {
    jarak = 0;
  } else if (jarak > 100) {
    jarak = 100;
  }

  newJarak = jarak;

  Serial.println("--------- SENSOR JARAK -----");
  Serial.print("Jarak : ");
  Serial.print(jarak);
  Serial.println(" cm");
}

void kirim_database() {
  if ((millis() - lastTime) > timerDelay) {
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      USE_SERIAL.print("[HTTP] Memulai...\n");
      
      http.begin( wiFiClient, urlSaveData + (String) newJarak + "&flame=" + flameKet );
      
      USE_SERIAL.print("[HTTP] Kirim ke database ...\n");
      int httpCode = http.GET();
  
      if(httpCode > 0)
      {
        USE_SERIAL.printf("[HTTP] kode response GET : %d\n", httpCode);
  
        if (httpCode == HTTP_CODE_OK) // kode 200 
        {
          respon = http.getString();
          USE_SERIAL.println("Respon : " + respon);

          statusRelay = respon.substring(0,1);
          statusRelayDua = respon.substring(2);
          USE_SERIAL.println("Status Relay 1: " + statusRelay);
          USE_SERIAL.println("Status Relay 2: " + statusRelayDua);
        
          delay(200);
        }
      }
      else
      {
        USE_SERIAL.printf("[HTTP] GET data gagal, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    
    lastTime = millis();
  }
}
