
int pin = 15;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pin, INPUT_PULLDOWN);
  
}

void loop() {
  int v_state = digitalRead(pin);

  if(v_state == HIGH){
    Serial.println("pulsado");
    }else{
      Serial.println("NO");
      }
}
