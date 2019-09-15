#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include "Secrets.h"

/* Secrets.h file should contain data as below: */
#ifndef WIFI_SSID
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define URL "http://doorbellreceiver.lan/ring?batt="
#endif


#define LED_PIN 0
#define RELAY_PIN 2
#define OTA_HOSTNAME "Doorbell-Transmitter"
int battVoltage;
HTTPClient http;
WiFiClient wClient;
WiFiClientSecure wClientSecure;
ADC_MODE(ADC_VCC);

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.begin(115200);
  pinMode(3, FUNCTION_3);
  pinMode(LED_PIN, OUTPUT);
  setupWifi();
  bool httpSucceeded = false;
  battVoltage = ESP.getVcc();
  String url = String(URL) + String(battVoltage);
  for (int j = 0; j < 5; j++) {
    if (postHTTP(url)) {
      httpSucceeded = true;
      break;
    }
    delay(2000); // wait 2 seconds before we retry
  }
  if (httpSucceeded) {
    Serial.println("HTTP Succeeded!");
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("HTTP Failed!");
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
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Powering off!");
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(1000);
  Serial.println("Since there is still power, we are moving into loop() and waiting for OTA...");
  digitalWrite(LED_PIN, HIGH);
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();

}


void loop() {
  ArduinoOTA.handle();
}


bool postHTTP(String url) {
  http.begin(wClient, url);
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
