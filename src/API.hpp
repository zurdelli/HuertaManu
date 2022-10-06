#include "ESP8266_Utils_APIREST.hpp"

float horariosFloat[7];
int diasArray[7];
int contDias = 0, contHorarios = 0;
float minutos = 0;
bool encendidoManual = false, estado = false;
int rele = D6;

String daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
String monthsNames[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};


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

bool enciendeBomba(DateTime date)
{
  int weekDay = date.dayOfTheWeek(); // Domingo 0 / Lunes 1 / Martes 2 / Miercoles 3 / Jueves 4 / Viernes 5 / Sabado 6
  float hours = date.hour() + date.minute()/100.0;

  bool hourCondition;
  bool dayCondition;
  float misMinutos = minutos / 100;



  // Mira el array contDias y verifica dia por dia
  for (int j=0; j < contDias; j++){
    dayCondition += (weekDay == diasArray[j]);
    //if(dayCondition) Serial.println(dayCondition);
  }

  // Mira el array contHorarios y verifica horario por horario
  for (int i = 0; i < contHorarios;i++){
    if ((horariosFloat[i]+misMinutos) - (int)horariosFloat[i] >= .60 ){
      float minutos = (horariosFloat[i]+misMinutos) - (int)horariosFloat[i] - .60;
      float horas = (int)horariosFloat[i] + 1;
      horariosFloat[i] = horas+minutos;
      Serial.println(horariosFloat[i]);
    }
    hourCondition += (hours >= horariosFloat[i] && hours < 
      (horariosFloat[i]+misMinutos));
    // if (hourCondition){
    //   Serial.print("HourCondition: ");
    //   Serial.println(hourCondition);
    // }
  }
  
  return (hourCondition && dayCondition) ? true : false;
}

void handleHorarios(String horas){
  horas.replace("\"","");
  horas.replace("[","");
  horas.replace("]","");

  horariosFloat[7]= {};

  String horaTmp = "";
  for (unsigned int i = 0; i < horas.length() ;i++){
    if (horas.charAt(i) == ','){
      horariosFloat[contHorarios] = horaTmp.toFloat();
      Serial.println(horariosFloat[contHorarios]);
      contHorarios++;
      horaTmp = "";
    } else {
      horaTmp+= horas.charAt(i);
    }
  }
  horariosFloat[contHorarios] = horaTmp.toFloat();
  Serial.println(horariosFloat[contHorarios]);
  contHorarios++;
  horaTmp = "";
}

void handleDias(String dias){

  diasArray[7] = {};

  for (int i = 0; i < dias.length() ; i++){
    diasArray[contDias] = dias.charAt(i)- '0';
    Serial.print("Dia agregado: ");
    Serial.println(diasArray[contDias]);
    contDias++;
  }
}

void setData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String bodyContent = GetBodyContent(data, len);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) { request->send(400); return;}

  int horaRTC = (rtc.now().unixtime());
  int horaJs = doc["horaActual"];
  if (horaJs - horaRTC > 60){
    rtc.adjust(DateTime(horaJs));
    Serial.println("Hora actualizada con éxito");
  }

  handleHorarios(doc["horas"]);
  handleDias(doc["dias"]);
  minutos = doc["minutos"];

  request->send(200);
}

void getData(AsyncWebServerRequest *request)
{
   AsyncResponseStream *response = request->beginResponseStream("application/json");
    
   // obtendríamos datos de GPIO, estado...
   StaticJsonDocument<300> jsonDoc;

  String horariosString = "";
   for(int i = 0; i < contHorarios;i++){
      horariosString += horariosFloat[i];
      horariosString += ", ";
   }
   String diasString="";
   for (int j=0; j< contDias; j++){
      diasString += diasArray[j];
      diasString += ", ";
   }
   jsonDoc["horas"] = horariosString;
   jsonDoc["minutos"] = minutos;
   jsonDoc["dias"] = diasString;
   serializeJson(jsonDoc, *response);
   
   request->send(response);
}

void toggleBombaManual(AsyncWebServerRequest * request){
  Serial.println("Toggle Bomba Manual");
  encendidoManual ? digitalWrite(rele, HIGH) : digitalWrite(rele, LOW);
  encendidoManual = !encendidoManual;
}

void handleBomba(){
  DateTime now = rtc.now(); // Obtener fecha y hora actual
  printDate(now);
  if(!encendidoManual){
    if (!estado && enciendeBomba(now)){
      digitalWrite(rele, LOW);
      estado = true;
      Serial.println("Bomba Encendida");
    } else if (estado && !enciendeBomba(now)) {
      digitalWrite(rele, HIGH);
      estado = false;
      Serial.print("Bomba Apagada");
    }
  }

}