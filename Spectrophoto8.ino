#define SCPI_ARRAY_SYZE 2            //Max branches of the command tree and max number of parameters.
#define SCPI_MAX_TOKENS 30           //Max number of valid tokens.
#define SCPI_MAX_COMMANDS 30         //Max number of registered commands.
#define SCPI_MAX_SPECIAL_COMMANDS 0  //Max number of special commands.
#define SCPI_BUFFER_LENGTH 128       // Length of the message buffer.
#define SCPI_HASH_TYPE uint8_t       // Integer size used for hashes.

#include "Arduino.h"
#include "AccelStepper.h"
#include "Vrekrer_scpi_parser.h"
#include "Constants.h"
#include "Config.h"
#include "Temperature.h"

#define STEPPER_STEP_PIN      2
#define STEPPER_DIR_PIN       5
#define STEPPER_ENABLE_PIN    8  // Active high
#define HOME_SENSOR_POWER_PIN A0
#define HOME_SENSOR_PIN       A1  // Active LOW sensor
#define NTC_PIN               A2
#define DOOR_PIN              A3
#define HEATER_MOSFET_PIN     6
#define LAMP_MOSFET_PIN       13
#define FAN_MOSFET_PIN        7

AccelStepper stepper(AccelStepper::DRIVER, STEPPER_STEP_PIN, STEPPER_DIR_PIN);
long currentStepPosition = 0;  // Absolute tracker (microsteps)

const char SW_REVISION[] = "3.7";

// Define global config variable
Config config;

extern SCPI_Parser my_instrument; 

//Function prototypes from SCPI
extern void SCPIInit();

bool carouselInitialized = false;
bool temperatureControl = false;
bool lampEnabled = false;
int carouselInitStatus = 0;

void applyMotionConfig() {
  stepper.setMaxSpeed(config.max_speed);
  stepper.setAcceleration(config.acceleration);
  stepper.setSpeed(config.step_speed);
}

void setup() {
  Serial.begin(115200);

  _delay_ms(1000);

  Serial.println(F("Spectrophotometer mechanical subasembly starting up..."));

  ReadConfigFromEEPROM();

  if (strncmp(config.sw_version, SW_REVISION, sizeof(config.sw_version)) != 0) {
    Serial.println(F("Software version mismatch."));
    ResetDefaultsEEPROM();
  } else {
    Serial.println(F("Software version matches."));
  }

  if (!ValidateCRC()) {
    Serial.println(F("CRC mismatch. Resetting to defaults."));
    ResetDefaultsEEPROM();
    ValidateCRC();
  } else {
    Serial.println(F("EEPROM CRC is valid."));
  }
  
  SCPIInit();

  tempInit();
  Serial.print("Current chamber temperature: ");
  print_temp(6);

  pinMode(FAN_MOSFET_PIN, OUTPUT);
  pinMode(HEATER_MOSFET_PIN, OUTPUT);
  pinMode(LAMP_MOSFET_PIN, OUTPUT);
  pinMode(DOOR_PIN, INPUT);

  pinMode(STEPPER_ENABLE_PIN, OUTPUT);
  digitalWrite(STEPPER_ENABLE_PIN, LOW); // Enable stepper driver

  applyMotionConfig();

  print_config();

  
  Serial.println("Initializing carousel...");
  carouselInitStatus = initializeCarousel();

  Serial.println(F("Type help to display all SCPI commands."));
}

void loop() {
  my_instrument.ProcessInput(Serial, "\n");
}
