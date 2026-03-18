#ifndef PID_H
#define PID_H

struct PidState {
  float temperatureC;
  float setpointC;
  float error;
  float integral;
  float derivative;
  float outputPct;
  bool heaterOn;
  bool fanOn;
  bool fault;
  bool validTemperature;
};

void heating(bool value);
void serviceTemperatureControl();
void getPidState(PidState &state);
void resetPidState();
void setMotionThermalInterlock(bool active);

#endif
