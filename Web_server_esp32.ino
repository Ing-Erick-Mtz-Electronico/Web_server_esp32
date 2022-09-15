
// Código para la bolla
//Lib Servidor
#include <AsyncEventSource.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

//Lib Wifi
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

#include <SPIFFS.h> //acceder al sistem file
 
//lib SPI
#include <SPI.h>
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

#include <Wire.h>
#include <RTClib.h> //lib RTC

//variables para el sensor PH
#define addressPH 99//0x63              //default I2C ID number for EZO pH Circuit.
byte codePH = 0;                   //used to hold the I2C response code.
char ph_data[20];               //we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_charPH = 0;                //used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte iPH = 0;                      //counter used for ph_data array.
int time_PH = 900;                 //used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.
float ph_float;                 //float var used to hold the float value of the RTD.
//---------------------

//variables para el sensor EC conductividad
#define addressEC 0x64              //default I2C ID number for EZO EC Circuit.
byte codeEC = 0;                   //used to hold the I2C response code.
char ec_data[20];               //we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_charEC = 0;                //used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte iEC = 0;                      //counter used for ec_data array.
int time_EC = 600;                 //used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.600
float ec_float;                 //float var used to hold the float value of the RTD
//---------------------------

//variables para el sensor RTC Temperatura
#define addressRTD 0x66              
byte codeRTD = 0;                   
char rtd_data[20];               
byte in_charRTD = 0;                
byte iRTD = 0;                     
int time_RTD = 600;                
float rtd_float;                 
//---------------------------


// Declaramos un RTC DS3231
RTC_DS1307 rtc;
DateTime now;
char str[20];
String fecha_data = "";

#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define PATH ("/prueba7.json")
#define BME_ADDRESS (0x76)

Adafruit_BME280 bme;

String temperature, humidity, pressure, altitude, PH, EC, RTD;
int counter = 0;

// Creamos nuestra propia red -> SSID & Password
const char* ssid = "GIDEAMSERVER";  
const char* password = "1234567890";

//Servidor
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

String sensorPH(){
  boolean flag = true;
  
  Wire.beginTransmission(addressPH);                                              //call the circuit by its ID number.
  Wire.write("R \n");                                                     //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                                       //end the I2C data transmission.

  while (flag){
    
    delay(time_PH);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(addressPH, 20, 1);                                           //call the circuit and request 20 bytes (this may be more than we need)
    codePH = Wire.read();
    
    switch (codePH) {                       //switch case based on what the response code is.
      case 1:                             //decimal 1.
        Serial.println("Success");        //means the command was successful.
        while (Wire.available()) {            //are there bytes to receive.
          in_charPH = Wire.read();              //receive a byte.
          ph_data[iPH] = in_charPH;              //load this byte into our array.
          iPH += 1;                             //incur the counter for the array element.
          if (in_charPH == 0) {                 //if we see that we have been sent a null command.
            iPH = 0;                            //reset the counter i to 0.
            break;                            //exit the while loop.
          }
        }
        
        //Serial.println("pH: " + String(ph_data));             //print the data.
        
        return String(ph_data);
                                
      case 2:                             //decimal 2.
        Serial.println("Failed");         //means the command has failed.
        return String("0.000");
        
  
      case 254:                           //decimal 254.
        Serial.println("Pending");        //means the command has not yet been finished calculating.
        break;                            //exits the switch case.
  
      case 255:                           //decimal 255.
        Serial.println("No Data");        //means there is no further data to send.
        return String("0.000");
    }
    } 
  }

String sensorEC(){
  boolean flag = true;
  
  Wire.beginTransmission(addressEC);                                              //call the circuit by its ID number.
  Wire.write("R \n");                                                     //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                                       //end the I2C data transmission.

  while (flag){
    
    delay(time_EC);                                                               //wait the correct amount of time for the circuit to complete its instruction.
  
    Wire.requestFrom(addressEC, 20, 1);                                           //call the circuit and request 20 bytes (this may be more than we need)
    codeEC = Wire.read();                                                         //the first byte is the response code, we read this separately.
  
    switch (codeEC) {                       //switch case based on what the response code is.
      case 1:                             //decimal 1.
        Serial.println("Success");        //means the command was successful.
        while (Wire.available()) {            //are there bytes to receive.
          in_charEC = Wire.read();              //receive a byte.
          ec_data[iEC] = in_charEC;              //load this byte into our array.
          iEC += 1;                             //incur the counter for the array element.
          if (in_charEC == 0) {                 //if we see that we have been sent a null command.
            iEC = 0;                            //reset the counter i to 0.
            break;                            //exit the while loop.
          }
        }
        return String(ec_data);
      case 2:                             //decimal 2.
        Serial.println("Failed");         //means the command has failed.
        return String("0.000");                            //exits the switch case.
  
      case 254:                           //decimal 254.
        Serial.println("Pending");        //means the command has not yet been finished calculating.
        break;                            //exits the switch case.
  
      case 255:                           //decimal 255.
        Serial.println("No Data");        //means there is no further data to send.
        return String("0.000");
    }
  
    //Serial.println("EC: " + String(ec_data));             //print the data.
    } 
  }

String sensorRTD(){
  boolean flag = true;
  
  Wire.beginTransmission(addressRTD);                                              //call the circuit by its ID number.
  Wire.write("R \n");                                                     //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                                       //end the I2C data transmission.

  while (flag){
    
    delay(time_RTD);                                                               //wait the correct amount of time for the circuit to complete its instruction.
  
    Wire.requestFrom(addressRTD, 20, 1);                                           //call the circuit and request 20 bytes (this may be more than we need)
    codeRTD = Wire.read();                                                         //the first byte is the response code, we read this separately.
  
    switch (codeRTD) {                       //switch case based on what the response code is.
      case 1:                             //decimal 1.
        Serial.println("Success");        //means the command was successful.
        while (Wire.available()) {            //are there bytes to receive.
          in_charRTD = Wire.read();              //receive a byte.
          rtd_data[iRTD] = in_charRTD;              //load this byte into our array.
          iRTD += 1;                             //incur the counter for the array element.
          if (in_charRTD == 0) {                 //if we see that we have been sent a null command.
            iRTD = 0;                            //reset the counter i to 0.
            break;                            //exit the while loop.
          }
        }
        return String(rtd_data);
      case 2:                             //decimal 2.
        Serial.println("Failed");         //means the command has failed.
        return String("0.000");                            //exits the switch case.
  
      case 254:                           //decimal 254.
        Serial.println("Pending");        //means the command has not yet been finished calculating.
        break;                            //exits the switch case.
  
      case 255:                           //decimal 255.
        Serial.println("No Data");        //means there is no further data to send.
        return String("0.000");
    }
  
    //Serial.println("EC: " + String(ec_data));             //print the data.
    } 
  }
//----------------------------------------------

void setup() {

  String init = "{\"mediciones\":[";
  
  Serial.begin(115200);
  while(!Serial) {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }
  Wire.begin();                 //enable I2C port.
  
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
  

  server.on("/lib/highstock/highstock.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/highstock.js");
  });
  server.on("/lib/highstock/data.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/data.js");
  });
  server.on("/lib/highstock/accessibility.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/accessibility.js");
  });
  server.on("/lib/highstock/data.js.map", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lib/highstock/data.js.map");
  });
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  delay(150);
  
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
    PH = sensorPH();
    EC = sensorEC();
    RTD = sensorRTD();
    
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
    Serial.print("ph = ");
    Serial.println(PH);
    Serial.print("EC = ");
    Serial.println(EC);
    Serial.print("RTD = ");
    Serial.println(RTD);

    
    String object_med = "{";
    object_med.concat("\"tambiente\":");
    object_med.concat(temperature);
    object_med.concat(",\"humedad\":");
    object_med.concat(humidity);
    object_med.concat(",\"presion\":");
    object_med.concat(pressure);
    object_med.concat(",\"altitud\":");
    object_med.concat(altitude);
    object_med.concat(",\"ph\":");
    object_med.concat(PH);
    object_med.concat(",\"conductividad\":");
    object_med.concat(EC);
    object_med.concat(",\"tagua\":");
    object_med.concat(RTD);
    
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
