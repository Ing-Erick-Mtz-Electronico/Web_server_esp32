
#include <AsyncEventSource.h>

#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

 

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>





#include <SPIFFS.h>



//#include <WebServer.h>

//#include <ESPAsyncWebServer.h>

//#include <WiFi.h>
//#include <WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

#include <Wire.h>
#include <RTClib.h>
 
// Declaramos un RTC DS3231
RTC_DS1307 rtc;
DateTime now;
char str[20];
String fecha_data = "";

// Código para la bolla

#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define PATH ("/prueba6.json")
#define BME_ADDRESS (0x76)

Adafruit_BME280 bme;

String temperature, humidity, pressure, altitude;
int counter = 0;

// Creamos nuestra propia red -> SSID & Password
const char* ssid = "GIDEAMSERVER";  
const char* password = "1234567890";

//Servidor
//WebServer server(80);
AsyncWebServer server(80);

//Millis
unsigned long previousMillis = 0;
const long intervalo = 10000;

//---------funciones----------------------------

String leerArchivo(fs::FS &fs, const char * path){
  String payload;
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(file){
    while(file.available()){
      char ch = file.read();
      payload += String(ch);
    }
    file.close();
    int tamanio = payload.length();
    payload[tamanio-1] = ']';
    payload += '}';
    Serial.println("enviados--------------------------");
    //Serial.println(payload);
    return payload;
  } else {
    Serial.println("Failed to open file for reading");
    return "{\"message\":\"error\"}";
  } 
}

boolean checkChar(fs::FS &fs, const char * path,const char ch){
  File file = fs.open(path);
  
  if(!file){
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file for check char: ");
  while(file.available()){
    char mychar = file.read();
    if(mychar == ch){
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

boolean checkFile(fs::FS &fs, const char * path){
  
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    file.close();
    return false;
  }
  
  Serial.printf("verificando archivo: %s\n", path);
  if(file.available()){
    file.close();
    return true;
  }
   
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void writeFile(fs::FS &fs, const char * path,const String mensaje) {
  Serial.printf("Escribiendo el archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escribirlo");
    return;
  }
  if (file.print(mensaje)) {
    Serial.println("Archivo escrito");
  } else {
    Serial.println("Error al escribir");
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);
  //String reed = "";
  File file = fs.open(path);
  
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    //char mychar = file.read();
    //reed.concat(mychar);
    Serial.write(file.read());
  }
  //Serial.println(reed);
  
  file.close();
  //return reed;
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

String readBME280Temperature() {
  // Read temperature as Celsius (the default)
  float t = bme.readTemperature();
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readBME280Humidity() {
  float h = bme.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readBME280Pressure() {
  float p = bme.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}
String readBME280Altitude() {
  float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
  
  if (isnan(a)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(a);
    return String(a);
  }
}
String fecha() {
  now = rtc.now();
  sprintf(str, "\"%02d-%02d-%02dT%02d:%02d:%02d\"",  now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  Serial.println(str);
  return str;
  
}
/*
//----------respuesta de rutas--------------
void inicio() {
  server.send(SPIFFS, "/index.html");
}
void tiempoReal() {
  server.send(SPIFFS, "/tiempo_real.html");
}

void tiemporealTemperature() {
  server.send(200, "text/plain", readBME280Temperature().c_str());
}
void tiemporealHumidity() {
  server.send(200, "text/plain", readBME280Humidity().c_str());
}
void tiemporealPressure() {
  server.send(200, "text/plain", readBME280Pressure().c_str());
}
void tiemporealAltitude() {
  server.send(200, "text/plain", readBME280Altitude().c_str());
}
void historial() {
  server.send(SPIFFS, "/historial.html");
}
void historialDatos() {
  String leer = leerArchivo(SD, PATH);
  server.send(200, "text/plain",leer.c_str());
}

void handleNotFound(){
  server.send(404, "text/plain", "{\"message\":\"URL failed\"}");
}
*/
//----------------------------------------------

void setup() {

  String init = "{\"mediciones\":[";
  
  Serial.begin(115200);
  while(!Serial) {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }

  // Creamos el punto de acceso
  WiFi.softAP(ssid, password); // Tiene mas parametros opcionales
  IPAddress ip = WiFi.softAPIP();
  Serial.println(ip);
  Serial.print("Nombre de mi red esp32: ");
  Serial.println(ssid);

  
  //Servicios
/*
  //-----------------------------------------
  server.on("/", inicio);
  
  server.on("tiemporeal/",tiempoReal);

  server.on("tiemporeal/temperature/",tiemporealTemperature);
  server.on("tiemporeal/humidity/",tiemporealHumidity);
  server.on("tiemporeal/pressure/",tiemporealPressure);
  server.on("tiemporeal/altitude/",tiemporealAltitude);
  
  server.on("historial/",historial);
  server.on("historial/datos/",historialDatos);
  
  server.onNotFound(handleNotFound);
  //-----------------------------------------
  */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  
  
  server.on("/tiemporeal", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/tiempo_real.html");
  });
  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Temperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Pressure().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Altitude().c_str());
  });
  
  server.on("/historial", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/historial.html");
  });
  server.on("/variables", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",leerArchivo(SD, PATH).c_str());
  });
  
  server.on("/lib/bootstrap/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/bootstrap/css/bootstrap.min.css");
  });
  server.on("/lib/bootstrap/js/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/bootstrap/js/bootstrap.min.js");
  });
  server.on("/lib/highstock/highstock.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/highstock.js");
  });
  server.on("/lib/highstock/data.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/data.js");
  });
  server.on("/lib/highstock/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/highcharts.js");
  });
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  delay(150);
  
  //Iniciando servidor
  
  if(!SPIFFS.begin(true)){
    Serial.println("sistema de archivos SPIFFS fallida");
    delay(2000);
  }
  Serial.println("Inicialización SPIFFS lista.");
  
  Serial.print("Inicializando SD card...");
  while(!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    delay(2000);
  }
  Serial.println("Inicialización lista.");

  while(!rtc.begin()){
    Serial.println("No hay un módulo RTC conectado");
    delay(2000);
    }
  Serial.println("RTC conectada");
  
  while(!bme.begin(BME_ADDRESS)){
    Serial.println("No hay un módulo BME conectado");
    delay(2000);
    }
  Serial.println("BME conectado");
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  //deleteFile(SD, PATH);
  if(checkFile(SD, PATH)){
    if(!checkChar(SD, PATH, '[')){
      writeFile(SD, PATH, init.c_str());
    }
  }else{
    writeFile(SD, PATH, init.c_str());
  }
}

void loop() {
  //Manejo del servidor
  //server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo && counter <= 50) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    temperature = readBME280Temperature();
    humidity = readBME280Humidity();
    pressure = readBME280Pressure();
    altitude = readBME280Altitude();
    fecha_data = fecha();
    
    Serial.print("Temperatura = "); 
    Serial.println(temperature);
    Serial.print("Humedad = "); 
    Serial.println(humidity);
    Serial.print("Presion = ");
    Serial.println(pressure);
    Serial.print("Altitud = ");
    Serial.println(altitude);
    Serial.print("fecha = ");
    Serial.println(fecha_data);
    
    String object_med = "{";
    object_med.concat("\"temperatura\":");
    object_med.concat(temperature);
    object_med.concat(",\"humedad\":");
    object_med.concat(humidity);
    object_med.concat(",\"presion\":");
    object_med.concat(pressure);
    object_med.concat(",\"altitud\":");
    object_med.concat(altitude);
    
    object_med.concat(",\"fecha\":");
    object_med.concat(fecha_data);
    object_med.concat("}");
    object_med.concat(",");
    
    appendFile(SD, PATH, object_med.c_str());
    //readFile(SD, PATH);
    counter++;
  }  
  //delay(10000);
  
  //readFile(SD, PATH);
  //Serial.print("\n");
  
}
