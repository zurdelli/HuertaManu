#include <Arduino.h>

#include <Wire.h>
#include "SPI.h"
#include "RTClib.h"


#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

RTC_DS3231 rtc;

#include "API.hpp"
#include "server.hpp"

const char *ssid_AP = "HuertaManuAP";


void setup()
{
  delay(1000);

  Serial.begin(9600);
  LittleFS.begin();

  pinMode(rele,OUTPUT);
  digitalWrite(rele, HIGH);

  if (!rtc.begin()){
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }


  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_AP);
  Serial.print("AP dirección IP: ");
  Serial.println(WiFi.softAPIP());

  InitServer();

  // Si se ha perdido la corriente, fijar fecha y hora
  if (rtc.lostPower()) {
    // Fijar a fecha y hora de compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Fijar a fecha y hora específica. En el ejemplo, 21 de Enero de 2016 a las 03:00:00
    // rtc.adjust(DateTime(2016, 1, 21, 3, 0, 0));
  }

}

void loop()
{
  handleBomba();

  delay(3000);
}

  