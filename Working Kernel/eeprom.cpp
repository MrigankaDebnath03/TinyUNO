#include "eeprom.h"

void writeEEPROM(unsigned int address, byte data) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(address >> 8));    // MSB of address
    Wire.write((int)(address & 0xFF));    // LSB of address
    Wire.write(data);
    Wire.endTransmission();
    delay(5);  // Allow time for write cycle to complete
}

byte readEEPROM(unsigned int address) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(address >> 8));    // MSB of address
    Wire.write((int)(address & 0xFF));    // LSB of address
    Wire.endTransmission();
    
    Wire.requestFrom(EEPROM_ADDRESS, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;
}
