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


String horarios[5]; // creo array de horarios
float horariosFloat[5];
int contador = 0;


/**
 * Enciende la bomba
 * 
 * */
bool enciendeBomba(DateTime date)
{
  int weekDay = date.dayOfTheWeek(); // Domingo 0 / Lunes 1 / Martes 2 / Miercoles 3 / Jueves 4 / Viernes 5 / Sabado 6
  float hours = date.hour() + date.minute() / 60.0;
  Serial.println(hours);
  bool dayCondition = (weekDay == 5 || weekDay == 6 || weekDay == 0);
  bool hourCondition;

  for (int i = 0; i< contador;i++){
    hourCondition += (hours >= horariosFloat[i] && hours <= horariosFloat[i] + .05);
  }
  
  bool condicionhoraria = (hours >= 14.09);

  //return (condicionhoraria && dayCondition) ? true : false;
  return (condicionhoraria) ? true : false;
}

void compilePattern(){

  MatchState ms;
  ms.Target ("Testing: answer=42"); // String to be searched

  /**
   * Regexp: 
   * 1. Hours 
   * 2. Minutes to finish
   * 3. Days at week
   * */
  String regExp = "14.00,21.00;5;1,4,5";

  String matcher = ("[]");


}

// called for each match
void setHorarios  (const char * match,          // matching string (not null-terminated)
                      const unsigned int length,   // length of matching string
                      const MatchState & ms)      // MatchState in use (to get captures)
{

  String hora;
  for (int i = 0; i< length ; i++){
    hora += match[i];
  }

  horariosFloat[contador] = hora.toFloat();
  contador++;

}  // end of match_callback 



void pruebaSetup(){
  unsigned long count;
  char buf [100] =  "14.00,13.44;5;1,4,5";

  
  MatchState ms (buf); // match state object


  // // GlobalMatch itera sobre un pattern, llama al callback cada vez que encuentra lo 
  // // que busca y luego devuelve la cantidad de veces que encontro eso
  count = ms.GlobalMatch(("(%d%d).(%d%d)"), setHorarios);
  //                       // "[o%a]"
  //                       // "^[0-9]{2}.[0-9]{2}"
  //                       // "(o%a+)( )"

  // char result = ms.Match(("(%d%d).(%d%d)(%p)"),0);
  // if (result == REGEXP_MATCHED){
  //   String hola = ms.GetMatch(buf);
  //   Serial.print("esto devuelve getmatch: ");
  //   Serial.println(hola);
  // }

  // show results
  Serial.print (count );
  Serial.println(" matches. ");
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

  pruebaSetup();
  for(int i = 0; i< contador ; i++){
    Serial.print("Horario: ");
    Serial.println(horariosFloat[i]);
  }

  if (!rtc.begin())
  {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }


  LittleFS.begin();
  WiFi.mode(WIFI_AP);
  //WiFi.softAP(ssid_AP, password_AP);
  WiFi.softAP(ssid_AP);
  Serial.print("AP dirección IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html",String(), false, processor);
  });


    // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });
  
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
  //server.handleClient();

  if (!state && enciendeBomba(now)){
    digitalWrite(rele, LOW);
    state = true;
    Serial.println("Activado");
  } else if (state && !enciendeBomba(now)) {
    digitalWrite(rele, HIGH);
    state = false;
    Serial.print("Desactivado");
  }

  //printDate(now);

  delay(3000);
}

  