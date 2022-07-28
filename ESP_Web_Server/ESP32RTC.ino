// Librería para la comunicación I2C y la RTClib
#include <Wire.h>
#include <RTClib.h>
 
// Declaramos un RTC DS3231
RTC_DS1307 rtc;
DateTime now;
char str[20];
 
void setup () {
  Serial.begin(115200);
  while(!Serial) {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }
 
  delay(1000);
 
  // Comprobamos si tenemos el RTC conectado
  if (! rtc.begin()) {
    Serial.println("No hay un módulo RTC");
    while (1);
  }
   
  // Ponemos en hora, solo la primera vez, luego comentar y volver a cargar.
  // Ponemos en hora con los valores de la fecha y la hora en que el sketch ha sido compilado.
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
 
void loop () {
 now = rtc.now();
 sprintf(str, "%02d-%02d-%02dT%02d:%02d:%02d",  now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
 Serial.println(str);
 delay(3000);
}
