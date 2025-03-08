#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x50  // I2C address for the EEPROM

void writeEEPROM(unsigned int address, byte data);
byte readEEPROM(unsigned int address);

#endif
