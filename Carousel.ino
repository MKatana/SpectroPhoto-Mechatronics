#include "Arduino.h"
#include "AccelStepper.h"
#include "Constants.h"
#include "Config.h"
#include "Pins.h"
#include "Carousel.h"
#include "Door.h"
#include "PID.h"

static bool startMotionToAbsolute(long targetPosition) {
  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
    return false;
  }

  if (!carouselInitialized) {
    Serial.println("Carousel not initialized!");
    return false;
  }

  if (motionActive || stepper.distanceToGo() != 0) {
    Serial.println(F("BUSY"));
    return false;
  }

  setMotionThermalInterlock(true);
  stepper.moveTo(targetPosition);
  motionActive = true;
  while (stepper.distanceToGo() != 0) {
    stepper.run();

    if (processDoorInterrupt(false)) {
      currentStepPosition = stepper.currentPosition();
      motionActive = false;
      setMotionThermalInterlock(false);
      return false;
    }
  }

  currentStepPosition = stepper.currentPosition();
  motionActive = false;
  setMotionThermalInterlock(false);
  Serial.println(F("OK"));
  return true;
}

void serviceMotion() {
  // Normal carousel moves are intentionally blocking again so temperature
  // sampling cannot starve stepper servicing and create visible jerkiness.
}

bool isMotionActive() {
  return motionActive;
}

int initializeCarousel() {
  motionActive = false;
  setMotionThermalInterlock(true);

  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Unable to initialize carousel. Door is open.");
    carouselInitStatus = 4;
    setMotionThermalInterlock(false);
    return (4); // Door open
  }

  unsigned int vdrop=analogRead(HOME_SENSOR_POWER_PIN);
  if (vdrop < 200){
    Serial.print("Position sensor malfunction! Current too high! ADC0 value:");
    Serial.println(vdrop);
    carouselInitialized = false;
    carouselInitStatus = 2;
    setMotionThermalInterlock(false);
    return(2); // Sensor current too high

  } else if(vdrop > 400){
    Serial.print("Position sensor malfunction! Current too low! ADC0 value:");
    Serial.println(vdrop);
    carouselInitialized = false;
    carouselInitStatus = 3;
    setMotionThermalInterlock(false);
    return(3); // Sensor current too low
  }

  if (digitalRead(HOME_SENSOR_PIN) == HIGH) {
    stepper.moveTo(500);
    while (stepper.distanceToGo() != 0) {
      stepper.run();

      if (processDoorInterrupt(false)) {
        stepper.setSpeed(0);
        Serial.println(F("Unable to initialize carousel. Door is opened."));
        carouselInitialized = false;
        carouselInitStatus = 4;
        setMotionThermalInterlock(false);
        return 4;
      }
    }
    if (digitalRead(HOME_SENSOR_PIN) == HIGH) {
      Serial.println(F("Unable to initialize carousel! Motor stuck on home position"));
      carouselInitialized = false;
      carouselInitStatus = 5;
      setMotionThermalInterlock(false);
      return(5); // Motor jammed on home position
  }
}

  long initStartPosition = stepper.currentPosition();
  stepper.setSpeed(-500);

  while (true) {
    stepper.runSpeed();

    if (processDoorInterrupt(false)) {
      stepper.setSpeed(0);
      Serial.println(F("Unable to initialize carousel. Door is opened."));
      carouselInitialized = false;
      carouselInitStatus = 4;
      setMotionThermalInterlock(false);
      return 4;
    }

    if (labs(stepper.currentPosition() - initStartPosition) > MAX_INIT_POSITION) {
      stepper.setSpeed(0);
      Serial.println(F("Unable to initialize carousel!"));
      carouselInitialized = false;
      carouselInitStatus = 1;
      setMotionThermalInterlock(false);
      return 1;  // Failure
    }

    if (digitalRead(HOME_SENSOR_PIN) == HIGH) {  // Sensor active
      stepper.setSpeed(0);
      currentStepPosition = INIT_OFFSET + config.offset;
      stepper.setCurrentPosition(currentStepPosition);
      Serial.println(F("Carousel initialized."));
      carouselInitialized = true;
      motionActive = false;
      carouselInitStatus = 0;
      setMotionThermalInterlock(false);
      return 0;  // Success
    }
  }
}

void moveToCuvette(int index) {

  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
    return;
  }

  if (!carouselInitialized) {
    Serial.println("Carousel not initialized!");
    return;
  }

  long targetSteps = config.offset + (getCarouselPosition(index - 1) * MICROSTEPPING_FACTOR);

  currentStepPosition = stepper.currentPosition();
  long normCurrent = ((currentStepPosition % FULL_ROTATION_STEPS) + FULL_ROTATION_STEPS) % FULL_ROTATION_STEPS;
  long normTarget = ((targetSteps % FULL_ROTATION_STEPS) + FULL_ROTATION_STEPS) % FULL_ROTATION_STEPS;

  long delta = normTarget - normCurrent;

  if (delta > FULL_ROTATION_STEPS / 2) {
    delta -= FULL_ROTATION_STEPS;
  } else if (delta < -FULL_ROTATION_STEPS / 2) {
    delta += FULL_ROTATION_STEPS;
  }

  startMotionToAbsolute(stepper.currentPosition() + delta);
}

void moveToSlot(int slot) {
  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
    return;
  }

  if (!carouselInitialized) {
    Serial.println("Carousel not initialized!");
    return;
  }

  long targetSteps = config.offset + ((long)(slot - 1) * SLOT_SPACING_STEPS * MICROSTEPPING_FACTOR);
  startMotionToAbsolute(targetSteps);
}

void moveToStep(int step) {

  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
    return;
  }

  if (!carouselInitialized) {
    Serial.println("Carousel not initialized!");
    return;
  }

  startMotionToAbsolute(step);
}


int getCarouselPosition(int index) {

  if (digitalRead(DOOR_PIN) == HIGH) {
    Serial.println("Door is open.");
    return(0);
  }

  if (!carouselInitialized) {
    Serial.println("Carousel not initialized!");
    return(0);
  }

  int slot = index / CUVE_PER_SLOT;
  int offset = index % CUVE_PER_SLOT;
  return (slot * SLOT_SPACING_STEPS) + (offset * CUVE_SPACING_STEPS);
}
