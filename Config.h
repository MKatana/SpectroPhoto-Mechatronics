#ifndef CONFIG_H
#define CONFIG_H

// Packed config struct
struct __attribute__((packed)) Config {
  char sw_version[4];
  unsigned int max_speed;
  unsigned int acceleration;
  unsigned int step_speed;
  int offset;
  float kp;
  float ki;
  float kd;
  float set_temp;
};

#endif
