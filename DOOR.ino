#include "Arduino.h"
#include "Pins.h"
#include "Carousel.h"
#include "Door.h"
#include "PID.h"

constexpr unsigned long DOOR_DEBOUNCE_MS = 100;

volatile bool doorIsrFired = false;
volatile unsigned long doorIsrTimeMs = 0;

bool interrupted = false;
bool lastDoorOpenState = false;
bool resumeHeatingAfterDoorClose = false;

inline bool doorIsOpenFast() {
  return (PINC & _BV(PC3)) != 0;
}

void doorInterruptInit() {
  lastDoorOpenState = doorIsOpenFast();

  PCICR |= _BV(PCIE1);
  PCMSK1 |= _BV(PCINT11);
  PCIFR |= _BV(PCIF1);
}

ISR(PCINT1_vect) {
  if (!doorIsrFired) {
    doorIsrTimeMs = millis();
    doorIsrFired = true;
  }
}

bool processDoorInterrupt(bool allowReinitialize) {
  if (!doorIsrFired) {
    return false;
  }

  if (millis() - doorIsrTimeMs < DOOR_DEBOUNCE_MS) {
    return false;
  }

  noInterrupts();
  doorIsrFired = false;
  interrupts();

  bool doorIsOpen = doorIsOpenFast();

  if (doorIsOpen == lastDoorOpenState) {
    return false;
  }

  lastDoorOpenState = doorIsOpen;

  if (doorIsOpen) {
    Serial.println(F("Door opened"));
    resumeHeatingAfterDoorClose = temperatureControl;
    heating(false);
    stepper.stop();
    stepper.setSpeed(0);
    digitalWrite(STEPPER_ENABLE_PIN, HIGH);
    carouselInitialized = false;
    motionActive = false;
  } else {
    digitalWrite(STEPPER_ENABLE_PIN, LOW);
    Serial.println(F("Door closed"));
    if (allowReinitialize) {
      Serial.println(F("Initializing carousel..."));
      carouselInitStatus = initializeCarousel();
      if (resumeHeatingAfterDoorClose && carouselInitialized) {
        heating(true);
      }
    }
    if (!allowReinitialize || carouselInitialized) {
      resumeHeatingAfterDoorClose = false;
    }
  }

  interrupted = true;
  return doorIsOpen;
}
