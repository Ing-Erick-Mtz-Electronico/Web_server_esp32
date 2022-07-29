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


// Código para el nodo de Sistemas Inteligentes

// Librería para el sensor de temperatura del suelo
// #include <Adafruit_MAX31865.h>

//#include <WebServer.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define PATH ("/prueba4.json")
#define BME_ADDRESS (0x76)

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

/*Put your SSID & Password*/
//const char* ssid = "LAB PROCESOS";  // Enter SSID here
//const char* password = "Pr0c3s0sUn1m4gd4l3n@.2017#*";  //Enter Password here

//WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  String init = "{\"mediciones\":[";
  //server.on("/hola/", handle_OnConnect);
  //server.onNotFound(handle_NotFound);

  //server.begin();
  //Serial.println("HTTP server started");
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

String fecha() {
  now = rtc.now();
  sprintf(str, "\"%02d-%02d-%02dT%02d:%02d:%02d\"",  now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  Serial.println(str);
  return str;
  
}

void loop() {
  
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
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
  object_med.concat(String(temperature));
  object_med.concat(",\"humedad\":");
  object_med.concat(String(humidity));
  object_med.concat(",\"presion\":");
  object_med.concat(String(pressure));
  object_med.concat(",\"altitud\":");
  object_med.concat(String(altitude));
  object_med.concat(",\"fecha\":");
  object_med.concat(fecha_data);
  object_med.concat("}");
  object_med.concat(",");
  
  appendFile(SD, PATH, object_med.c_str());
  delay(10000);
  
  readFile(SD, PATH);
  Serial.print("\n");
  
}
