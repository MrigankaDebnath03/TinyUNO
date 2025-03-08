#include "filesystem.h"
#include <SPI.h>

const int chipSelect = 4; // Changed to 10, which is the standard CS pin for most Arduino SD card shields

bool initSDCard() {
    // Make sure the SD card chip select pin is set as an output
    pinMode(chipSelect, OUTPUT);
    
    // Try to initialize the SD card with explicit CS pin
    if (!SD.begin(chipSelect)) {
        Serial.println(F("SD Card initialization failed."));
        return false;
    }
    
    Serial.println(F("SD Card initialized successfully."));
    return true;
}

void createFile(const char *filename) {
    // Make sure SD is initialized first
    if (!initSDCard()) {
        Serial.println(F("Cannot create file - SD card not initialized."));
        return;
    }
    
    if (SD.exists(filename)) {
        Serial.print(F("File already exists: "));
        Serial.println(filename);
        return;
    }
    
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.print(F("This is the content of "));
        file.println(filename);
        file.close();
        Serial.print(F("Created "));
        Serial.println(filename);
    } else {
        Serial.println(F("Create failed - Could not open file for writing."));
    }
}

void listFiles() {
    // Make sure SD is initialized first
    if (!initSDCard()) {
        Serial.println(F("Cannot list files - SD card not initialized."));
        return;
    }
    
    File root = SD.open("/");
    if (!root) {
        Serial.println(F("Failed to open root directory."));
        return;
    }
    
    Serial.println(F("\n--- SD Card Contents ---"));
    
    // Count number of files
    int fileCount = 0;
    while (File entry = root.openNextFile()) {
        Serial.print(F("  "));
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.print(F(" (directory)"));
        } else {
            // Display file size
            Serial.print(F(" ("));
            Serial.print(entry.size());
            Serial.print(F(" bytes)"));
            fileCount++;
        }
        Serial.println();
        entry.close();
    }
    
    if (fileCount == 0) {
        Serial.println(F("  No files found."));
    }
    
    root.close();
    Serial.println(F("------------------------"));
}

void deleteFile(const char *filename) {
    // Make sure SD is initialized first
    if (!initSDCard()) {
        Serial.println(F("Cannot delete file - SD card not initialized."));
        return;
    }
    
    if (SD.exists(filename)) {
        if (SD.remove(filename)) {
            Serial.print(F("Deleted "));
            Serial.println(filename);
        } else {
            Serial.println(F("Delete failed - Permission denied."));
        }
    } else {
        Serial.print(F("File not found: "));
        Serial.println(filename);
    }
}