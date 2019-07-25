#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include "Secrets.h"

/* Secrets.h file should contain data as below: */
#ifndef WIFI_SSID
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define IFTTT_URL "http://maker.ifttt.com/trigger/doorbell/with/key/xxxxxxxxxxxxxxxxxxxxxx"
#define THINGSPEAK_KEY
#endif

#define LED_PIN 0
#define RELAY_PIN 2
#define MANUAL_BOOT_PIN 3
#define OTA_HOSTNAME "Doorbell-Transmitter"
bool manualBoot = false;
int batVoltage;
HTTPClient http;
WiFiClient wClient;
WiFiClientSecure wClientSecure;
ADC_MODE(ADC_VCC);

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.begin(115200);
  pinMode(3, FUNCTION_3);
  pinMode(MANUAL_BOOT_PIN, INPUT);
  if (digitalRead(MANUAL_BOOT_PIN) == LOW) {
    Serial.println("Manual boot!");
    manualBoot = true;
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.begin();
  }
  pinMode(LED_PIN, OUTPUT);
  batVoltage = ESP.getVcc();
  setupWifi();
  if (!manualBoot) {
    if (postToIfttt()) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Done!");
    } else {
      Serial.println("IFTTT did not work!");
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
    }
    postToThingSpeak();
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    Serial.println("Powering off!");
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}


void loop() {
  if (manualBoot) {
    ArduinoOTA.handle();
  }
}


bool postToIfttt() {
  http.begin(wClient, IFTTT_URL);
  uint httpCode = http.GET();
  http.end();
  if (httpCode == 200) {
    Serial.println(httpCode);
    return true;
  } else {
    Serial.println(httpCode);
    return false;
  }
}


void postToThingSpeak() {
  String url = "/update?api_key=" + String(THINGSPEAK_KEY) + "&field1=" + String(batVoltage);
  wClientSecure.setInsecure(); // until we have better handling of a trust chain on small devices
  http.begin(wClientSecure, "api.thingspeak.com", 443, url, true);
  uint httpCode = http.GET();
  Serial.println(httpCode);
  http.end();
}


void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_PIN, HIGH);
    delay(10);
    digitalWrite(LED_PIN, LOW);
    delay(10);
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(700);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}
