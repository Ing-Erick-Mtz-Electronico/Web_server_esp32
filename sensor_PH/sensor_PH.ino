

#include <Wire.h>
#define address 99//0x63              //default I2C ID number for EZO pH Circuit.

char computerdata[20];           //we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte received_from_computer = 0; //we need to know how many characters have been received.
byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
byte code = 0;                   //used to hold the I2C response code.
char ph_data[20];               //we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_char = 0;                //used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte i = 0;                      //counter used for ph_data array.
int time_ = 900;                 //used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.
float ph_float;                 //float var used to hold the float value of the RTD.

String PH;
//Millis
unsigned long previousMillis = 0;
const long intervalo = 3000;

String sensorPH(){
  boolean flag = true;
  
  Wire.beginTransmission(address);                                              //call the circuit by its ID number.
  Wire.write("R \n");                                                     //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                                       //end the I2C data transmission.

  while (flag){
    
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(address, 20, 1);                                           //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();
    
    switch (code) {                       //switch case based on what the response code is.
      case 1:                             //decimal 1.
        Serial.println("Success");        //means the command was successful.
        while (Wire.available()) {            //are there bytes to receive.
          in_char = Wire.read();              //receive a byte.
          ph_data[i] = in_char;              //load this byte into our array.
          i += 1;                             //incur the counter for the array element.
          if (in_char == 0) {                 //if we see that we have been sent a null command.
            i = 0;                            //reset the counter i to 0.
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial) {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }
  Wire.begin();                 //enable I2C port.
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    previousMillis = currentMillis;
    PH = sensorPH();
    Serial.print("ph = ");
    Serial.println(PH);
  }
}
