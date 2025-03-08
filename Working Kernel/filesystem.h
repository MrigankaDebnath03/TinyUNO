#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <SD.h>

// Function to initialize SD card
bool initSDCard();
void createFile(const char *filename);
void deleteFile(const char *filename);
void listFiles();

#endif