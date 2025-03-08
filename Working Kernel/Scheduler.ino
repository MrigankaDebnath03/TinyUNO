#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "scheduler.h"
#include "eeprom.h"
#include "distance_task.h"
#include "led_task.h"
#include "filesystem.h"
#include "bluetooth_transfer.h"  // Include Bluetooth transfers

void setup() {
    Serial.begin(9600);
    Wire.begin();
    scheduler_init();

    // Initialize hardware tasks
    setup_distance_sensor();
    setup_led();
    
    // Initialize SD Card
    if (!initSDCard()) {
        Serial.println(F("SD Card initialization failed!"));
    } else {
        Serial.println(F("SD Card initialized."));
    }
    
    // Initialize Bluetooth module for file transfers
    bt_init();

    // Register tasks
    scheduler_register_task("distance", distance_task_wrapper);
    scheduler_register_task("led", led_task_wrapper);

    // Display available commands
    Serial.println(F("Scheduler Started. Commands:"));
    Serial.println(F("  start - Start the scheduler"));
    Serial.println(F("  stop - Stop the scheduler"));
    Serial.println(F("  CREATE <filename> - Create a file (scheduler must be stopped)"));
    Serial.println(F("  DELETE <filename> - Delete a file (scheduler must be stopped)"));
    Serial.println(F("  VIEW - List files on SD card"));
    Serial.println(F("  exec <task> [-t duration] [-p priority] - Execute a task"));
    Serial.println(F("  halt <task> - Stop a task"));
    Serial.println(F("  inspect - View task status"));
    Serial.println(F("  BTGET - Receive file via Bluetooth (scheduler must be stopped)"));
    Serial.println(F("  BTSEND <filename> - Send file via Bluetooth (scheduler must be stopped)"));
    Serial.println(F("  BTDIAG - Run Bluetooth module diagnostic"));
}

void loop() {
    scheduler_handle_command();
    if (isSchedulerRunning()) {
        scheduler_run();
    }
}
