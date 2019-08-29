#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include "Secrets.h"

/* Secrets.h file should contain data as below: */
#ifndef WIFI_SSID
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define THINGSPEAK_KEY "xxxxxxxxxx"
#define APILIO_VARIABLE "variable"
#define APILIO_KEY "xxxxxxxxxxxxxxxxxxxxxx"
String URLS[] = {
                  "http://maker.ifttt.com/trigger/doorbell/with/key/xxxxxxxxxxxxxxxxxxxxxx",
                  "http://lab.grapeot.me/ifttt/delay?event={IFTTT_EVENT_NAME}&t={TIME_IN_MINUTES}&key={IFTTT_KEY}"
                };
/* Documentation for IFTTT delayed triggers: https://grapeot.me/adding-a-delay-to-ifttt-recipes.html */
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
  setupWifi();
  if (!manualBoot) {
    if (armed()) {
      Serial.println("System is armed! Continuing...");
      int numUrls = sizeof(URLS) / sizeof(URLS[0]);
      for (int i = 0; i < numUrls; i++) {
        bool httpSucceeded = false;
        for (int j = 0; j < 5; j++) {
          Serial.print("Requesting URL: ");
          Serial.print(URLS[i]);
          Serial.print(" - Status: ");
          if (postHTTP(URLS[i])) {
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
      }
    } else {
      Serial.println("System is not armed! Exiting...");
    }
    Serial.print("Sending battery voltage to ThingSpeak... ");
    batVoltage = ESP.getVcc();
    postToThingSpeak();
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    Serial.println("Powering off!");
    delay(10000);
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


void postToThingSpeak() {
  String url = "/update?api_key=" + String(THINGSPEAK_KEY) + "&field1=" + String(batVoltage);
  wClientSecure.setInsecure(); // until we have better handling of a trust chain on small devices
  http.begin(wClientSecure, "api.thingspeak.com", 443, url, true);
  uint httpCode = http.GET();
  Serial.println(httpCode);
  http.end();
}


bool armed() {
  Serial.print("Checking if Apilio status is true... Status: ");
  String auth = "Basic " + String(APILIO_KEY);
  String url = "/api/v1/boolean_variables/" + String(APILIO_VARIABLE);
  wClientSecure.setInsecure(); // until we have better handling of a trust chain on small devices
  http.begin(wClientSecure, "api.apilio.com", 443, url, true);
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", auth);
  uint httpCode = http.GET();
  String httpResponse = http.getString();
  Serial.println(httpCode);
  http.end();
  if (httpCode == 200) {
    int startIndex = httpResponse.indexOf("value") + 7;
    int endIndex = httpResponse.indexOf("\"", startIndex) - 1;
    String status = httpResponse.substring(startIndex, endIndex);
    if (status == "true") {
      return true;
    } else if (status == "false") {
      return false;
    } else {
      Serial.println("Returning true status due to invalid HTTP response!"); // graceful degradation
      Serial.println(httpResponse);
      return true;
    }
  } else {
    Serial.println("Returning true status due to HTTP error!"); // graceful degradation
    return true;
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
