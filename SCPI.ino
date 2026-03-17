#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"
#include "Constants.h"
#include "Config.h"
#include "Temperature.h"
#include <avr/wdt.h>

SCPI_Parser my_instrument;

// Prototypes
void Identify(SCPI_Commands, SCPI_Parameters, Stream&);
void ResetBoard(SCPI_Commands, SCPI_Parameters, Stream&);
void Help(SCPI_Commands, SCPI_Parameters, Stream&);
void GetDoor(SCPI_Commands, SCPI_Parameters, Stream&);
void SetLamp(SCPI_Commands, SCPI_Parameters, Stream&);
void GetLamp(SCPI_Commands, SCPI_Parameters, Stream&);
void SetTemp(SCPI_Commands, SCPI_Parameters, Stream&);
void GetTemp(SCPI_Commands, SCPI_Parameters, Stream&);
void SetTempCtrl(SCPI_Commands, SCPI_Parameters, Stream&);
void GetTempCtrl(SCPI_Commands, SCPI_Parameters, Stream&);
void GetKp(SCPI_Commands, SCPI_Parameters, Stream&);
void SetKp(SCPI_Commands, SCPI_Parameters, Stream&);
void GetKi(SCPI_Commands, SCPI_Parameters, Stream&);
void SetKi(SCPI_Commands, SCPI_Parameters, Stream&);
void GetKd(SCPI_Commands, SCPI_Parameters, Stream&);
void SetKd(SCPI_Commands, SCPI_Parameters, Stream&);
void ResetEEPROM(SCPI_Commands, SCPI_Parameters, Stream&);
void MoveToCuvette(SCPI_Commands, SCPI_Parameters, Stream&);
void MoveToSlot(SCPI_Commands, SCPI_Parameters, Stream&);
void MoveToStep(SCPI_Commands, SCPI_Parameters, Stream&);
void GetStepPos(SCPI_Commands, SCPI_Parameters, Stream&);
void SetOffset(SCPI_Commands, SCPI_Parameters, Stream&);
void GetOffset(SCPI_Commands, SCPI_Parameters, Stream&);
void SetMaxSpeed(SCPI_Commands, SCPI_Parameters, Stream&);
void GetMaxSpeed(SCPI_Commands, SCPI_Parameters, Stream&);
void SetAcceleration(SCPI_Commands, SCPI_Parameters, Stream&);
void GetAcceleration(SCPI_Commands, SCPI_Parameters, Stream&);
void SetSpeed(SCPI_Commands, SCPI_Parameters, Stream&);
void GetSpeed(SCPI_Commands, SCPI_Parameters, Stream&);
void StartInit(SCPI_Commands, SCPI_Parameters, Stream&);
void GetInit(SCPI_Commands, SCPI_Parameters, Stream&);


void SCPIInit() {
  my_instrument.hash_magic_number = 37;  //Default value = 37
  my_instrument.hash_magic_offset = 7;   //Default value = 7
  my_instrument.timeout = 10;            //value in miliseconds. Default value = 10

  // Register commands
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);  //works
  my_instrument.RegisterCommand(F("*RST"), &ResetBoard);
  my_instrument.RegisterCommand(F("HELP"), &Help);  //works
  my_instrument.RegisterCommand(F("DOOr?"), &GetDoor);  //works
  my_instrument.RegisterCommand(F("LAMp?"), &GetLamp);  //works
  my_instrument.RegisterCommand(F("LAMp"), &SetLamp);  //works
  my_instrument.RegisterCommand(F("TEMP?"), &GetTemp);  //works
  my_instrument.RegisterCommand(F("TEMP"), &SetTemp);  //works
  my_instrument.SetCommandTreeBase(F("TEMP"));
    my_instrument.RegisterCommand(F(":CONtrol"), &SetTempCtrl);  //NOT WORKING
    my_instrument.RegisterCommand(F(":CONtrol?"), &GetTempCtrl);  //NOT WORKING
    my_instrument.RegisterCommand(F(":Kp?"), &GetKp);  //works
    my_instrument.RegisterCommand(F(":Kp"), &SetKp);  //works
    my_instrument.RegisterCommand(F(":Ki?"), &GetKi);  //works
    my_instrument.RegisterCommand(F(":Ki"), &SetKi);  //works
    my_instrument.RegisterCommand(F(":Kd?"), &GetKd);  //works
    my_instrument.RegisterCommand(F(":Kd"), &SetKd);  //works
  my_instrument.SetCommandTreeBase(F("EEPROM"));
    my_instrument.RegisterCommand(F(":READ"), &ReadEEPROM);  //works
    my_instrument.RegisterCommand(F(":WRITE"), &WriteEEPROM);  //works
    my_instrument.RegisterCommand(F(":RESET"), &ResetEEPROM);  //works
  my_instrument.SetCommandTreeBase(F("POSition"));
    my_instrument.RegisterCommand(F(":CUVette"), &MoveToCuvette);  //works
    my_instrument.RegisterCommand(F(":SLOT"), &MoveToSlot); //NOT WORKING
    my_instrument.RegisterCommand(F(":STEP"), &MoveToStep); //works
    my_instrument.RegisterCommand(F(":STEP?"), &GetStepPos); //NOT WORKING
    my_instrument.RegisterCommand(F(":OFFset"), &SetOffset); //  working
    my_instrument.RegisterCommand(F(":OFFset?"), &GetOffset); //  working
    my_instrument.RegisterCommand(F(":MAXSPeed"), &SetMaxSpeed); //  working
    my_instrument.RegisterCommand(F(":MAXSPeed?"), &GetMaxSpeed); //  working
    my_instrument.RegisterCommand(F(":ACCeleration"), &SetAcceleration); //  working
    my_instrument.RegisterCommand(F(":ACCeleration?"), &GetAcceleration); //  working
    my_instrument.RegisterCommand(F(":SPEed"), &SetSpeed); //  working
    my_instrument.RegisterCommand(F(":SPEed?"), &GetSpeed); //  working
    my_instrument.RegisterCommand(F(":INITialize"), &StartInit); //  working
    my_instrument.RegisterCommand(F(":INITialize?"), &GetInit); //  not working
}

extern const char SW_REVISION[];  // declare the variable if defined elsewhere

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.print(F("Katana, Spectrophotometer Mechanical Subsystem, #01, "));
  Serial.println(SW_REVISION);
}

void ResetBoard(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(F("Resetting controller..."));
  Serial.flush();
  delay(10);
  wdt_enable(WDTO_15MS);

  while (true) {
  }
}

void ReadEEPROM(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  ReadConfigFromEEPROM();
}

void WriteEEPROM(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  WriteConfigToEEPROM();
}

void ResetEEPROM(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  ResetDefaultsEEPROM();
}

void MoveToCuvette(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int index = constrain(String(parameters[0]).toInt(), 1, 72);
  moveToCuvette(index);
}

void MoveToSlot(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int slot = constrain(String(parameters[0]).toInt(), 1, 6);
  moveToSlot(slot);
}

void MoveToStep(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int step = constrain(String(parameters[0]).toInt(), 0, FULL_ROTATION_STEPS);
  moveToStep(step);
}

void GetStepPos(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    Serial.println(stepper.currentPosition());
}

void SetOffset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.offset = constrain(String(parameters[0]).toInt(), -200, 200);
  Serial.println(config.offset);
}

void GetOffset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.offset);
}

void SetMaxSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.max_speed = constrain(String(parameters[0]).toInt(), 100, 10000);
  applyMotionConfig();
  Serial.println(config.max_speed);
}

void GetMaxSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.max_speed);
}

void SetAcceleration(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.acceleration = constrain(String(parameters[0]).toInt(), 100, 50000);
  applyMotionConfig();
  Serial.println(config.acceleration);
}

void GetAcceleration(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.acceleration);
}

void SetSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.step_speed = constrain(String(parameters[0]).toInt(), 100, 5000);
  applyMotionConfig();
  Serial.println(config.step_speed);
}

void GetSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.step_speed);
}

void StartInit(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(F("Reinitializing..."));
  carouselInitStatus = initializeCarousel();
}

void GetInit(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    Serial.println(carouselInitialized ? 1 : 0);
}

void GetDoor(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
  } else{
    Serial.println("Door is closed.");
  }
}

void SetLamp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int value = constrain(String(parameters[0]).toInt(), 0, 1);
  lamp(value == 1);
}

void GetLamp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(lampEnabled ? 1 : 0);
}

void GetTemp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  float tempC;

  if (!tempReadC(tempC)) {
    Serial.println(F("TEMP_READ_ERROR"));
    return;
  }

  Serial.println(tempC, 6);
}

void SetTemp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.set_temp=constrain(String(parameters[0]).toFloat(), 20.0, 40.0);
  Serial.print("Temperature setpoint: ");
  Serial.println(config.set_temp);
}

void SetTempCtrl(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int value = constrain(String(parameters[0]).toInt(), 0, 1);
  if (value == 1) {
    heating(1);
  } else
    heating(0);
}

void GetTempCtrl(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(temperatureControl ? 1 : 0);
}

void GetKp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.kp);
}

void SetKp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.kp = constrain(String(parameters[0]).toFloat(), 0, 10);
}

void GetKi(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.ki);
}

void SetKi(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.ki = constrain(String(parameters[0]).toFloat(), 0, 10);
}

void GetKd(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(config.kd);
}

void SetKd(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  config.kd = constrain(String(parameters[0]).toFloat(), 0, 10);
}

void Help(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.println(F("\n"));
  Serial.println(F("*IDN? - Identify"));
  Serial.println(F("*RST - Reset controller board"));
  Serial.println(F("HELP - Print all SCPI commands and their explanations"));
  Serial.println(F("EEPROM"));
  Serial.println(F("   :READ - load parameters from EEPROM"));
  Serial.println(F("   :WRITE - save the current parameters to EEPROM"));
  Serial.println(F("   :RESET - reset EEPROM to default values"));
  Serial.println(F("POSition"));
  Serial.println(F("   :CUVette # - go to cuvette number (1-72)"));
  Serial.println(F("   :SLOT # - go to cuvette slot (1-6)"));
  Serial.println(F("   :STEP?  - read the current step position"));
  Serial.println(F("   :STEP # - go to motor step (0-10080)"));
  Serial.println(F("   :OFFSet? - read home sensor offset value"));
  Serial.println(F("   :OFFSet # - set offset from home sensor (-200-200)"));
  Serial.println(F("   :MAXSpeed? - read the max speed parameter"));
  Serial.println(F("   :MAXSpeed # - set max speed parameter (100-10000)"));
  Serial.println(F("   :ACCeleration? - read the acceleration parameter"));
  Serial.println(F("   :ACCeleration # - set acceleration parameter (100-50000)"));
  Serial.println(F("   :SPEED? - read the speed parameter"));
  Serial.println(F("   :SPEED # - set speed parameter (100-5000)"));
  Serial.println(F("   :INITialize - initialize carousel"));
  Serial.println(F("   :INITialize? - read initialization status")); //
  Serial.println(F("DOOR? - read door status"));
  Serial.println(F("LAMP? - read lamp status"));
  Serial.println(F("LAMP # - turn the lamp on or off"));
  Serial.println(F("TEMP"));
  Serial.println(F("    ? - read current temperature in Celsius"));
  Serial.println(F("    # - set temperature in Celsius (20-40)"));
  Serial.println(F("    :CONtrol? - read heating status"));
  Serial.println(F("    :CONtrol # - enable or disable heating (0-1)"));
  Serial.println(F("    :KP? - read P value"));
  Serial.println(F("    :KP # - set P value (0-10)"));
  Serial.println(F("    :KI? - read I value"));
  Serial.println(F("    :KI # - set I value (0-10)"));
  Serial.println(F("    :KD? - read D value"));
  Serial.println(F("    :KD # - set D value (0-10)"));
}
