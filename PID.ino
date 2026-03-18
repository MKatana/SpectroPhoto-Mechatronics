#include "Pins.h"
#include "Config.h"
#include "Temperature.h"
#include "PID.h"

constexpr unsigned long PID_SAMPLE_MS = 80;
constexpr unsigned long HEATER_WINDOW_MS = 1000;
constexpr unsigned long PID_REPORT_MS = 1000;
constexpr float PID_INTEGRAL_MIN = -100.0f;
constexpr float PID_INTEGRAL_MAX = 100.0f;
constexpr bool PID_SERIAL_DEBUG = false;

extern Config config;
extern bool temperatureControl;

static unsigned long lastPidSampleMs = 0;
static unsigned long heaterWindowStartMs = 0;
static unsigned long lastPidReportMs = 0;
static float pidIntegral = 0.0f;
static float lastTempC = 0.0f;
static float lastPidError = 0.0f;
static float lastPidDerivative = 0.0f;
static float heaterOutputPct = 0.0f;
static bool lastTempValid = false;
static bool temperatureFault = false;
static bool heaterState = false;
static bool fanState = false;
static bool motionThermalInterlock = false;

static void applyThermalOutputs(bool heaterOn, bool fanOn) {
  heaterState = heaterOn;
  fanState = fanOn;
  digitalWrite(HEATER_MOSFET_PIN, heaterOn ? HIGH : LOW);
  digitalWrite(FAN_MOSFET_PIN, fanOn ? HIGH : LOW);
}

static void resetTemperatureController() {
  pidIntegral = 0.0f;
  heaterOutputPct = 0.0f;
  lastTempValid = false;
  temperatureFault = false;
  lastPidSampleMs = 0;
  heaterWindowStartMs = millis();
  lastPidReportMs = 0;
  lastPidError = 0.0f;
  lastPidDerivative = 0.0f;
}

void resetPidState() {
  resetTemperatureController();
}

void setMotionThermalInterlock(bool active) {
  motionThermalInterlock = active;

  if (active) {
    applyThermalOutputs(false, temperatureControl);
  } else if (!temperatureControl) {
    applyThermalOutputs(false, false);
  } else {
    // Resume in a safe state; the next control update will re-assert demand.
    applyThermalOutputs(false, true);
  }
}

void heating(bool value) {
  if (temperatureControl == value) {
    return;
  }

  temperatureControl = value;
  resetTemperatureController();

  if (value) {
    applyThermalOutputs(false, true);
    Serial.println(F("Heating on"));
  } else {
    applyThermalOutputs(false, false);
    Serial.println(F("Heating off"));
  }
}

void serviceTemperatureControl() {
  const unsigned long now = millis();

  if (!temperatureControl) {
    applyThermalOutputs(false, false);
    return;
  }

  if (motionThermalInterlock) {
    applyThermalOutputs(false, true);
    return;
  }

  if (heaterWindowStartMs == 0 || (now - heaterWindowStartMs) >= HEATER_WINDOW_MS) {
    heaterWindowStartMs = now;
  }

  if (lastPidSampleMs == 0 || (now - lastPidSampleMs) >= PID_SAMPLE_MS) {
    const unsigned long sampleDtMs = (lastPidSampleMs == 0) ? PID_SAMPLE_MS : (now - lastPidSampleMs);
    const float dt = sampleDtMs / 1000.0f;
    float tempC;

    lastPidSampleMs = now;

    if (!tempReadC(tempC)) {
      applyThermalOutputs(false, false);
      heaterOutputPct = 0.0f;
      pidIntegral = 0.0f;
      lastTempValid = false;

      if (!temperatureFault) {
        Serial.println(F("TEMP_READ_ERROR"));
        temperatureFault = true;
      }

      return;
    }

    if (temperatureFault) {
      pidIntegral = 0.0f;
      temperatureFault = false;
    }

    const float error = config.set_temp - tempC;
    const float derivative = lastTempValid ? ((tempC - lastTempC) / dt) : 0.0f;
    const float proportionalTerm = config.kp * error;
    const float derivativeTerm = config.kd * derivative;
    const float unclampedOutput = proportionalTerm + (config.ki * pidIntegral) - derivativeTerm;
    const bool outputPinnedHigh = unclampedOutput >= 100.0f;
    const bool outputPinnedLow = unclampedOutput <= 0.0f;
    const bool shouldIntegrate = !((outputPinnedHigh && (error > 0.0f)) ||
                                   (outputPinnedLow && (error < 0.0f)));

    if (shouldIntegrate) {
      pidIntegral += error * dt;

      if (pidIntegral > PID_INTEGRAL_MAX) {
        pidIntegral = PID_INTEGRAL_MAX;
      } else if (pidIntegral < PID_INTEGRAL_MIN) {
        pidIntegral = PID_INTEGRAL_MIN;
      }
    }

    float output = proportionalTerm + (config.ki * pidIntegral) - derivativeTerm;

    if (output > 100.0f) {
      output = 100.0f;
    } else if (output < 0.0f) {
      output = 0.0f;
    }

    heaterOutputPct = output;
    lastTempC = tempC;
    lastPidError = error;
    lastPidDerivative = derivative;
    lastTempValid = true;
  }

  const bool heaterOn = !temperatureFault &&
                        ((now - heaterWindowStartMs) < (unsigned long)(HEATER_WINDOW_MS * (heaterOutputPct / 100.0f)));

  applyThermalOutputs(heaterOn, true);

  if (PID_SERIAL_DEBUG && ((lastPidReportMs == 0) || ((now - lastPidReportMs) >= PID_REPORT_MS))) {
    lastPidReportMs = now;
    Serial.print(F("PID T="));
    if (lastTempValid) {
      Serial.print(lastTempC, 3);
    } else {
      Serial.print(F("NaN"));
    }
    Serial.print(F(" SP="));
    Serial.print(config.set_temp, 3);
    Serial.print(F(" E="));
    Serial.print(lastPidError, 3);
    Serial.print(F(" I="));
    Serial.print(pidIntegral, 3);
    Serial.print(F(" D="));
    Serial.print(lastPidDerivative, 3);
    Serial.print(F(" OUT="));
    Serial.print(heaterOutputPct, 1);
    Serial.print(F("% H="));
    Serial.print(heaterState ? 1 : 0);
    Serial.print(F(" F="));
    Serial.print(fanState ? 1 : 0);
    Serial.print(F(" FAULT="));
    Serial.println(temperatureFault ? 1 : 0);
  }
}
