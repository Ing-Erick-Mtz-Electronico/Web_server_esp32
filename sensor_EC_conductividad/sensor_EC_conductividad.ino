#include <Wire.h>                //enable I2C.
#define addressEC 0x64              //default I2C ID number for EZO EC Circuit.



char computerdata[20];           //we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte received_from_computer = 0; //we need to know how many characters have been received.
byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
byte code = 0;                   //used to hold the I2C response code.
char ec_data[20];               //we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_char = 0;                //used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte i = 0;                      //counter used for ec_data array.
int time_ = 600;                 //used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.600
float ec_float;                 //float var used to hold the float value of the RTD.


String sensorEC(){
  boolean flag = true;
  
  Wire.beginTransmission(addressEC);                                              //call the circuit by its ID number.
  Wire.write("R \n");                                                     //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                                       //end the I2C data transmission.

  while (flag){
    
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
  
    Wire.requestFrom(addressEC, 20, 1);                                           //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.
  
    switch (code) {                       //switch case based on what the response code is.
      case 1:                             //decimal 1.
        Serial.println("Success");        //means the command was successful.
        while (Wire.available()) {            //are there bytes to receive.
          in_char = Wire.read();              //receive a byte.
          ec_data[i] = in_char;              //load this byte into our array.
          i += 1;                             //incur the counter for the array element.
          if (in_char == 0) {                 //if we see that we have been sent a null command.
            i = 0;                            //reset the counter i to 0.
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

void setup() {
  Serial.begin(115200);           //enable serial port.
  Wire.begin();                 //enable I2C port.Wire.beginTransmission(address);
}

void loop() {
  Serial.println("EC: " + sensorEC());

}
