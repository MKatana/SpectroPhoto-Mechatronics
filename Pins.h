#ifndef PINS_H
#define PINS_H

#include "Arduino.h"

constexpr uint8_t STEPPER_STEP_PIN = 2;
constexpr uint8_t STEPPER_DIR_PIN = 5;
constexpr uint8_t STEPPER_ENABLE_PIN = 8;
constexpr uint8_t HOME_SENSOR_POWER_PIN = A0;
constexpr uint8_t HOME_SENSOR_PIN = A1;
constexpr uint8_t NTC_PIN = A2;
constexpr uint8_t DOOR_PIN = A3;
constexpr uint8_t HEATER_MOSFET_PIN = 6;
constexpr uint8_t LAMP_MOSFET_PIN = 13;
constexpr uint8_t FAN_MOSFET_PIN = 7;

#endif
