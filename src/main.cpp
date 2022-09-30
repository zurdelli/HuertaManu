#include <Arduino.h>

#include <Wire.h>
#include "SPI.h"
#include "RTClib.h"

#include <Regexp.h>

#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ESP8266WebServer.h>


// RTC_DS1307 rtc;
RTC_DS3231 rtc;

String daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
String monthsNames[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

const int rele = D0;
String releState;
bool state = false;

const char *ssid_AP = "ESP8266_AP";
const char *password_AP = "12345678";

AsyncWebServer server(80);

float horariosFloat[5];
int diasArray[6];
int contDias = 0, contHorarios = 0;
float minutos = 0;



/**
 * Enciende la bomba
 * 
 * */
bool enciendeBomba(DateTime date)
{
  int weekDay = date.dayOfTheWeek(); // Domingo 0 / Lunes 1 / Martes 2 / Miercoles 3 / Jueves 4 / Viernes 5 / Sabado 6
  float hours = date.hour() + date.minute()/100.0;

  bool hourCondition;
  bool dayCondition;
  float misMinutos = minutos / 100;


  
  for (int j=0; j < contDias; j++){
    //Serial.print("Day condition: ");
    dayCondition += (weekDay == diasArray[j]);
    //if(dayCondition) Serial.println(dayCondition);
  }

  for (int i = 0; i < contHorarios;i++){
    //Serial.println(hours);
    //Serial.println(horariosFloat[i]);
    //Serial.println(horariosFloat[i] + misMinutos);
    hourCondition += (hours >= horariosFloat[i] && hours < 
      (horariosFloat[i]+misMinutos));
    if (hourCondition){
      Serial.print("HourCondition: ");
      Serial.println(hourCondition);
    }
  }
  
  return (hourCondition && dayCondition) ? true : false;
}


void setHorarios  (const char * match,          // matching string (not null-terminated)
                      const unsigned int length,   // length of matching string
                      const MatchState & ms)      // MatchState in use (to get captures)
{
  String hora;

  // 7 por la palabra "horarios"
  // -2 por que tuve que poner ";m" al final de la linea ya que sino me daba eror
  // tuve que repetir el codigo fuera del for ya que sino no me capturaba el ultimo horario
  for (int i = 7; i< length -2; i++){
    if (match[i] != ','){
      hora += match[i];
    } else {
      //Serial.println(hora);
      horariosFloat[contHorarios] = hora.toFloat();
      contHorarios++;
      hora = "";
    }
  }

  //Serial.println(hora);
  horariosFloat[contHorarios] = hora.toFloat();
  contHorarios++;
  hora = "";
}

void setMinutos (const char * match, const unsigned int length, const MatchState & ms){
  String minuto;
  // Mismo funcionamiento que el anterior, 6 ya que "minuto" tiene 6 letras
  for(int i = 6; i < length; i++){
    minuto += match[i];
  }
  //Serial.print("Minutos: ");
  minutos = minuto.toFloat();
  //Serial.println(minutos);
}

void setDias (const char * match, const unsigned int length, const MatchState & ms){
  String dias;

  for(int i = 4; i < length; i++){
    dias += match[i];
    
    // Se debe restar un '0' en char para que quede el numeero limpio
    if(match[i] - '0' >= 0 && (int)match[i] - '0' <= 9){
      //Serial.println("Dia agregado");
      diasArray[contDias] = match[i]- '0';
      contDias++;
    }
  }
}

void handleSave(AsyncWebServerRequest *request){
  String horas = request->arg("horas");
  Serial.print("Horas: ");
  //Serial.println(horas);

  String hora ="";

  for (int i = 0; i < horas.length() ;i++){
    if (horas.charAt(i) != ','){
      hora+= horas.charAt(i);
    } else {
      Serial.println(hora);
      horariosFloat[contHorarios] = hora.toFloat();
      contHorarios++;
      hora = "";  
    }
  }
  Serial.println(hora);
  horariosFloat[contHorarios] = hora.toFloat();
  contHorarios++;
  hora = "";

  String tiempo = request->arg("tiempo");
  Serial.print("Minutos: ");
  //Serial.println(tiempo);


  String minutosString = "";

  for (int i = 0; i < tiempo.length(); i++){
    minutosString += tiempo.charAt(i);
  }

  minutos = minutosString.toFloat();
  Serial.println(minutos);

  String dias = request->arg("dias");
  Serial.print("dias: ");
  //Serial.println(dias);

  String diasString = "";

  for (int i = 0; i < dias.length() ; i++){
    diasString += dias.charAt(i);

    if (dias.charAt(i) - '0' >= 0 && dias.charAt(i) - '0' <= 9){
      diasArray[contDias] = dias.charAt(i)- '0';
      Serial.print("Dia agregado: ");
      Serial.println(diasArray[contDias]);
      contDias++;
    }
  }

  request->redirect("/");

}

void pruebaSetup(){
  unsigned long count;

  char buf [100] =  "horario12.54,14.05,16.04;minuto5;dias1,4,5";
  MatchState ms (buf); // match state object

  count = ms.GlobalMatch(("horario.*;m"), setHorarios);
  // Serial.print (count );
  // Serial.println(" matches. ");
  int count2 = ms.GlobalMatch(("minuto%d"), setMinutos);
  int count3 = ms.GlobalMatch(("dias.*$"), setDias);
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

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(rele)){
      releState = "ON";
    }
    else{
      releState = "OFF";
    }
    Serial.print(releState);
    return releState;
  } else return releState;
  //return String();
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  pinMode(rele, OUTPUT);

  if (!rtc.begin()){
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }

  //pruebaSetup();

  LittleFS.begin();
  WiFi.mode(WIFI_AP);
  //WiFi.softAP(ssid_AP, password_AP);
  WiFi.softAP(ssid_AP);
  Serial.print("AP dirección IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html",String(), false, processor);
  });

  server.on("/save", HTTP_POST, handleSave);


    // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });
  
  server.begin();
  Serial.println("Servidor inicializado.");

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

  DateTime now = rtc.now(); // Obtener fecha y hora actual

  if (!state && enciendeBomba(now)){
    digitalWrite(rele, LOW);
    state = true;
    Serial.println("Activado");
  } else if (state && !enciendeBomba(now)) {
    digitalWrite(rele, HIGH);
    state = false;
    Serial.print("Desactivado");
  }

  printDate(now);

  delay(3000);
}

  