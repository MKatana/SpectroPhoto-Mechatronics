void heating(bool value){
  temperatureControl = value;
  if(value==true){
    digitalWrite(HEATER_MOSFET_PIN, HIGH);
    digitalWrite(FAN_MOSFET_PIN, HIGH);
    Serial.println("Heating on");
  }else{
    digitalWrite(HEATER_MOSFET_PIN, LOW);
    digitalWrite(FAN_MOSFET_PIN, LOW);
    Serial.println("Heating off");
  }
}
