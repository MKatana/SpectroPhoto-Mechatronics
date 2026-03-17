#include <Wire.h>
#include <math.h>

#define ADC_ADDR 0x68

const float VDIV = 3.30;
const float R_FIXED = 4700.0;
const float R0 = 10000.0;
const float BETA = 3950.0;
const float T0 = 298.15;
const float LSB = 62.5e-6;

const unsigned long ADC_TIMEOUT_MS = 100;

bool tempSensorPresent = false;

bool adcPresent() {
  Wire.beginTransmission(ADC_ADDR);
  return (Wire.endTransmission() == 0);
}

bool tempInit() {
  Wire.begin();

  if (!adcPresent()) {
    Serial.println(F("ERROR: MCP3421 not detected"));
    tempSensorPresent = false;
    return false;
  }

  Serial.println(F("MCP3421 detected"));

  // Continuous conversion, 16-bit, gain = 1
  Wire.beginTransmission(ADC_ADDR);
  Wire.write(0x18);

  if (Wire.endTransmission() != 0) {
    Serial.println(F("ERROR: MCP3421 configuration failed"));
    tempSensorPresent = false;
    return false;
  }

  tempSensorPresent = true;
  return true;
}

bool readADC(long &value) {
  unsigned long t0 = millis();

  while (millis() - t0 < ADC_TIMEOUT_MS) {
    Wire.requestFrom((uint8_t)ADC_ADDR, (uint8_t)3);

    if (Wire.available() != 3) {
      delay(1);
      continue;
    }

    long raw = 0;

    raw |= (long)Wire.read() << 8;
    raw |= (long)Wire.read();

    byte config = Wire.read();

    if ((config & 0x80) == 0) {
      if (raw & 0x8000) {
        raw |= 0xFFFF0000;
      }

      value = raw;
      return true;
    }

    delay(1);
  }

  return false;
}

bool tempReadC(float &tempC) {
  long raw;

  if (!tempSensorPresent) {
    return false;
  }

  if (!readADC(raw)) {
    return false;
  }

  float v = raw * LSB;

  if (v <= 0.0 || v >= 2.048) {
    return false;
  }

  float r_ntc = R_FIXED * (VDIV / v - 1.0);
  float tempK = 1.0 / ((1.0 / T0) + log(r_ntc / R0) / BETA);

  tempC = tempK - 273.15;
  return true;
}

void print_temp(int dec) {
  float tempC;

  if (!tempReadC(tempC)) {
    Serial.println(F("TEMP_READ_ERROR"));
    return;
  }

  Serial.println(tempC, dec);
}
