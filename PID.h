#ifndef PID_H
#define PID_H

void heating(bool value);
void serviceTemperatureControl();
void resetPidState();
void setMotionThermalInterlock(bool active);

#endif
