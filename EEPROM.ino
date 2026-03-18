#include "EEPROM.h"
#include "CRC32.h"
#include "Constants.h"
#include "Config.h"

// Declare global variable defined elsewhere (in main.ino)
extern Config config;

const int EEPROM_CRC_ADDR = EEPROM.length() - sizeof(uint32_t);

void WriteConfigToEEPROM() {
  EEPROM.put(0, config);
  uint32_t crc = CRC32::calculate((uint8_t*)&config, sizeof(Config));
  EEPROM.put(EEPROM_CRC_ADDR, crc);

  Serial.print(F("CRC written: 0x"));
  Serial.println(crc, HEX);
}

void ReadConfigFromEEPROM() {
  Serial.println(F("Loading values from EEPROM..."));
  EEPROM.get(0, config);
}

bool ValidateCRC() {
  // Buffer to hold raw EEPROM bytes for Config
  uint8_t buffer[sizeof(Config)];

  // Read raw bytes from EEPROM into buffer
  for (int i = 0; i < sizeof(Config); i++) {
    buffer[i] = EEPROM.read(i);
  }

  // Compute CRC over EEPROM bytes (Config structure)
  uint32_t computed_crc = CRC32::calculate(buffer, sizeof(Config));

  // Read stored CRC from EEPROM
  uint32_t stored_crc;
  EEPROM.get(EEPROM_CRC_ADDR, stored_crc);

  Serial.print(F("Stored CRC: 0x"));
  Serial.println(stored_crc, HEX);
  Serial.print(F("Computed CRC: 0x"));
  Serial.println(computed_crc, HEX);

  // Compare and return validation result
  return computed_crc == stored_crc;
}

void ResetDefaultsEEPROM() {
  Serial.println(F("Writing default values to EEPROM..."));
  memset(&config, 0, sizeof(Config));
  strncpy(config.sw_version, SW_REVISION, sizeof(config.sw_version));
  config.max_speed = DEFAULT_MAX_SPEED;
  config.acceleration = DEFAULT_ACCELERATION;
  config.step_speed = DEFAULT_STEP_SPEED;
  config.offset = DEFAULT_OFFSET;
  config.slot_offset = DEFAULT_SLOT_OFFSET;
  config.kp = DEFAULT_KP;
  config.ki = DEFAULT_KI;
  config.kd = DEFAULT_KD;
  config.set_temp = DEFAULT_SET_TEMP;

  WriteConfigToEEPROM();
}
