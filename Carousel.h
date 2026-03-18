#ifndef CAROUSEL_H
#define CAROUSEL_H

#include "Arduino.h"
#include "AccelStepper.h"
#include "Config.h"

constexpr int MICROSTEPPING_FACTOR = 4;
constexpr int FULL_STEPS_PER_REV = 2520;
constexpr int FULL_ROTATION_STEPS = FULL_STEPS_PER_REV * MICROSTEPPING_FACTOR;
constexpr int CUVE_PER_SLOT = 12;
constexpr int SLOT_COUNT = 6;
constexpr int SLOT_SPACING_STEPS = 420;
constexpr int CUVE_SPACING_STEPS = 30;
constexpr int INIT_OFFSET = 0;
constexpr int MAX_INIT_POSITION = 11000;

extern AccelStepper stepper;
extern long currentStepPosition;
extern Config config;
extern bool carouselInitialized;
extern bool motionActive;
extern int carouselInitStatus;

int initializeCarousel();
void serviceMotion();
bool isMotionActive();
void moveToCuvette(int index);
void moveToSlot(int slot);
void moveToStep(int step);
int getCarouselPosition(int index);

#endif
