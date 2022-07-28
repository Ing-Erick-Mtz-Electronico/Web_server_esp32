#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

// Código para el nodo de Sistemas Inteligentes

// Librería para el sensor de temperatura del suelo
// #include <Adafruit_MAX31865.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define PATH ("/prueba3.json")
#define BME_ADDRESS 0x76

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;
int counter = 0;

// Creamos nuestra propia red -> SSID & Password
const char* ssid = "GIDEAMSERVER";  
const char* password = "UN1M4GD4L3N4";

//Servidor
WebServer server(80);

//Millis
unsigned long previousMillis = 0;
const long intervalo = 10000;

void setup() {
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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("tiemporeal/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/tiempo_real.html");
  });
  
  server.on("tiemporeal/temperature/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Temperature().c_str());
  });
  server.on("tiemporeal/humidity/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });
  server.on("tiemporeal/pressure/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Pressure().c_str());
  });
  server.on("tiemporeal/altitude/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Altitude().c_str());
  });
  
  server.on("historial/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/historial.html");
  });
  server.on("historial/datos/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain",);
  });
  
  //server.on("historial/datos/", handleDevice);
  
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain","no found");
  });
  
  //Iniciando servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  delay(150);

  bme.begin(BME_ADDRESS);
  
  boolean band = false;
  String init = "{\"mediciones\":[";

  Serial.print("Inicializando SD card...");
  while(!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    delay(2000);
  }
  Serial.println("Inicialización lista.");
  
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
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo && counter <= 50) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    
    Serial.print("Temperatura = "); 
    Serial.println(temperature);
    Serial.print("Humedad = "); 
    Serial.println(humidity);
    Serial.print("Presion = ");
    Serial.println(pressure);
    Serial.print("Altitud = ");
    Serial.println(altitude);
    
    String object_med = "{";
    object_med.concat("\"temperatura\":");
    object_med.concat(String(temperature));
    object_med.concat(",\"humedad\":");
    object_med.concat(String(humidity));
    object_med.concat(",\"presion\":");
    object_med.concat(String(pressure));
    object_med.concat(",\"altitud\":");
    object_med.concat(String(altitude));
    object_med.concat("}");
    object_med.concat(",");
    
    appendFile(SD, PATH, object_med.c_str());
    counter++;
  }  
  //delay(10000);
  
  //readFile(SD, PATH);
  //Serial.print("\n");
  
}

// Responder a la url-------------------------

void handleDevice() {
  String payload = leerArchivo(SD, PATH);
  server.send(200, "text/plain", payload);
}

void handleNotFound(){
  server.send(404, "text/plain", "Not found");
}
// --------------------------------------------

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
    return payload;
  } else {
    Serial.println("Failed to open file for reading");
    return "Error leyendo archivo";
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
