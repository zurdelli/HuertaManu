#include <Arduino.h>
#include <Wire.h>
#include "SPI.h"
#include "RTClib.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// RTC_DS1307 rtc;
RTC_DS3231 rtc;

String daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
String monthsNames[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

const int rele = D0;
bool state = false;

const char *ssid_AP = "ESP8266_AP";
const char *password_AP = "12345678";
ESP8266WebServer server(80);

// Comprueba si esta programado el encendido
bool isScheduledON(DateTime date)
{
  int weekDay = date.dayOfTheWeek(); // Domingo 0 / Lunes 1 / Martes 2 / Miercoles 3 / Jueves 4 / Viernes 5 / Sabado 6
  float hours = date.hour() + date.minute() / 60.0;
  bool hourCondition = (hours > 14.00 && hours < 14.05) || (hours > 21.00 && hours < 23.00);
  bool dayCondition = (weekDay == 5 || weekDay == 6 || weekDay == 0);

  return (hourCondition && dayCondition) ? true : false;
}

void printDate(DateTime date)
{
  Serial.print(date.year(), DEC);
  Serial.print('/');
  Serial.print(date.month(), DEC);
  Serial.print('/');
  Serial.print(date.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[date.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(date.hour(), DEC);
  Serial.print(':');
  Serial.print(date.minute(), DEC);
  Serial.print(':');
  Serial.print(date.second(), DEC);
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  if (!rtc.begin())
  {
    Serial.println(F("Couldn't find RTC"));
    while (1)
      ;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_AP, password_AP);
  Serial.println("WiFi conectada.");
  Serial.println();
  WiFi.printDiag(Serial);
  Serial.println();
  Serial.print("AP dirección IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", []()
            { server.send(200, "text/plain", "Hola mundo!!"); });
  server.begin();
  Serial.println("Servidor inicializado.");

  // Si se ha perdido la corriente, fijar fecha y hora
  if (rtc.lostPower())
  {
    // Fijar a fecha y hora de compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Fijar a fecha y hora específica. En el ejemplo, 21 de Enero de 2016 a las 03:00:00
    // rtc.adjust(DateTime(2016, 1, 21, 3, 0, 0));
  }
}

void loop()
{

  DateTime now = rtc.now(); // Obtener fecha y hora actual
  server.handleClient();

  if (!state && isScheduledON(now))
  {
    digitalWrite(rele, HIGH);
    state = true;
    Serial.println("Activado");
  }
  else if (state && !isScheduledON(now))
  {
    digitalWrite(rele, LOW);
    state = false;
    Serial.print("Desactivado");
  }

  // printDate(now);

  delay(3000);
}

  