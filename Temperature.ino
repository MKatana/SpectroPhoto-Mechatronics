#include "Oversampling.h"

const float R_FIXED = 10000.0;    // ohms
const float R0 = 10000.0;         // NTC nominal resistance at T0 (ohms)
const float BETA = 3850.0;        // Beta constant (K)
const float T0 = 25.0 + 273.15;   // 25°C in Kelvin
const int ADC_BITS = 16;          // Number of ADC bits
const int AVG_SAMPLES = 1;        // Number of averaged samples

Oversampling oversampledADC(10, ADC_BITS, AVG_SAMPLES);

float temp_read(){

unsigned int adcValue = oversampledADC.read(NTC_PIN);

  float v = adcValue / (pow(2,ADC_BITS)-1); // Resistor ratio value from 0 to 1
  float R_NTC = R_FIXED * v / (1.0 - v);  // R = Rf * v/(1-v)
  float invT = (1.0 / T0) + (1.0 / BETA) * log(R_NTC / R0);
  float T_kelvin = 1.0 / invT;
  float T_celsius = T_kelvin - 273.15;
  return T_celsius;
}

void print_temp(int dec){
  float temp=temp_read();
  Serial.println(temp, dec);
}