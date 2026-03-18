#include "Pins.h"

void lamp(bool value){
  lampEnabled = value;
  if(value==true){
    digitalWrite(LAMP_MOSFET_PIN, HIGH);
    Serial.println("Lamp ON");
  }else{
    digitalWrite(LAMP_MOSFET_PIN, LOW);
    Serial.println("Lamp OFF");
  }
}
