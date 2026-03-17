void print_config(){
  Serial.println(F("\n--- Loaded Configuration ---"));
  Serial.print(F("SW_REVISION: ")); Serial.println(config.sw_version);
  Serial.print(F("MAX_SPEED: ")); Serial.println(config.max_speed);
  Serial.print(F("ACCELERATION: ")); Serial.println(config.acceleration);
  Serial.print(F("STEP_SPEED: ")); Serial.println(config.step_speed);
  Serial.print(F("OFFSET: ")); Serial.println(config.offset);
  Serial.print(F("KP: ")); Serial.println(config.kp);
  Serial.print(F("KI: ")); Serial.println(config.ki);
  Serial.print(F("KD: ")); Serial.println(config.kd);
  Serial.print(F("SET_TEMP: ")); Serial.println(config.set_temp);
}