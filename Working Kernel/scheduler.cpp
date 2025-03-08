
#include "scheduler.h"
#include "eeprom.h"
#include "filesystem.h"
#include "bluetooth_transfer.h"
#include <Wire.h>
#include <string.h>

ScheduledTask taskList[MAX_REGISTERED_TASKS];
int taskCount = 0;
int activeTaskCount = 0;
bool isPaused = false;
char commandBuffer[CMD_BUFFER_SIZE];

void scheduler_init() {
    taskCount = 0;
    activeTaskCount = 0;
    isPaused = false;
    memset(commandBuffer, 0, CMD_BUFFER_SIZE);
}

bool isSchedulerRunning() {
    return !isPaused;
}

void scheduler_register_task(const char* name, TaskFunction function) {
    if (taskCount < MAX_REGISTERED_TASKS) {
        strncpy(taskList[taskCount].name, name, 9);
        taskList[taskCount].name[9] = '\0'; // Ensure null-termination
        taskList[taskCount].function = function;
        taskList[taskCount].duration = DEFAULT_DURATION;
        taskList[taskCount].startTime = 0;
        taskList[taskCount].endTime = 0;
        taskList[taskCount].priority = 0;
        taskList[taskCount].active = false;
        taskList[taskCount].swapped = false;
        taskCount++;
    }
}

void scheduler_add_task(const char* name, unsigned long duration, int priority) {
    int regIndex = -1;
    for (int i = 0; i < taskCount; i++) {
        if (strcmp(taskList[i].name, name) == 0) {
            regIndex = i;
            break;
        }
    }
    if (regIndex == -1) {
        Serial.print(F("Task function not found for: "));
        Serial.println(name);
        return;
    }
    taskList[regIndex].duration = duration;
    taskList[regIndex].priority = priority;
    if (taskList[regIndex].active) {
        Serial.print(F("Task already active: "));
        Serial.println(name);
        return;
    }
    // If there is free RAM slot, mark task as active
    if (activeTaskCount < MAX_TASKS) {
        taskList[regIndex].active = true;
        taskList[regIndex].swapped = false;
        taskList[regIndex].startTime = 0;
        activeTaskCount++;
        Serial.print(F("Added task: "));
        Serial.println(name);
    } else {
        // Memory is full; choose the lowest-priority active task to swap out
        int lowestIndex = -1;
        int lowestPriority = 99999;
        for (int i = 0; i < taskCount; i++) {
            if (taskList[i].active && taskList[i].priority < lowestPriority) {
                lowestPriority = taskList[i].priority;
                lowestIndex = i;
            }
        }
        if (lowestIndex != -1) {
            Serial.println(F("Memory Full: Swapping out the lowest priority task..."));
            swap_out_task(lowestIndex);
            activeTaskCount--;  // one active task removed
            // Activate the new task
            taskList[regIndex].active = true;
            taskList[regIndex].swapped = false;
            taskList[regIndex].startTime = 0;
            activeTaskCount++;
            Serial.print(F("Added task: "));
            Serial.println(name);
        } else {
            Serial.println(F("No active task available to swap out."));
        }
    }
}

void scheduler_remove_task(const char* name) {
    for (int i = 0; i < taskCount; i++) {
        if (strcmp(taskList[i].name, name) == 0) {
            if (taskList[i].active) {
                taskList[i].active = false;
                taskList[i].swapped = false;
                activeTaskCount--;
                Serial.print(F("Removing task: "));
                Serial.println(name);
            } else {
                Serial.print(F("Task not active: "));
                Serial.println(name);
            }
            return;
        }
    }
    Serial.println(F("Task not found."));
}

void scheduler_run() {
    if (isPaused || taskCount == 0) return;
    unsigned long currentMillis = millis();
    static int currentTaskIndex = 0;
    if (currentTaskIndex >= taskCount) currentTaskIndex = 0;
    ScheduledTask* task = &taskList[currentTaskIndex];

    // If the task is marked swapped, swap it in first
    if (!task->active && task->swapped) {
        swap_in_task(currentTaskIndex);
        activeTaskCount++;
    }
    // If first run in this cycle, set start/end times
    if (task->startTime == 0) {
        task->startTime = currentMillis;
        task->endTime = currentMillis + task->duration;
    }
    // Execute the task function
    task->function();
    // If the task's time slot is over, cycle it out
    if (currentMillis >= task->endTime) {
        Serial.print(F("Cycling out task: "));
        Serial.println(task->name);
        swap_out_task(currentTaskIndex);
        activeTaskCount--;
        // Move to next task in round-robin order
        currentTaskIndex = (currentTaskIndex + 1) % taskCount;
        // For the new current task, if it's swapped, swap it in
        if (taskList[currentTaskIndex].swapped) {
            swap_in_task(currentTaskIndex);
            activeTaskCount++;
        }
        // Reset its timing for a new cycle
        taskList[currentTaskIndex].startTime = 0;
    }
}

void scheduler_inspect() {
    isPaused = true;
    Serial.println(F("\n--- Task List ---"));
    for (int i = 0; i < taskCount; i++) {
        Serial.print(F("Name: "));
        Serial.print(taskList[i].name);
        Serial.print(F(" | Duration: "));
        Serial.print(taskList[i].duration);
        Serial.print(F("ms | Priority: "));
        Serial.print(taskList[i].priority);
        Serial.print(F(" | Active: "));
        Serial.print(taskList[i].active ? F("Yes") : F("No"));
        Serial.print(F(" | Swapped: "));
        Serial.println(taskList[i].swapped ? F("Yes") : F("No"));
    }
    Serial.println(F("-----------------"));
}

void scheduler_handle_command() {
    // Do not process new commands if a BT transfer is active
    if (btTransferActive) return;

    static int bufferIndex = 0;
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || bufferIndex >= CMD_BUFFER_SIZE - 1) {
            commandBuffer[bufferIndex] = '\0';
            bufferIndex = 0;
            char cmd[10];
            char taskName[20];
            unsigned long duration = DEFAULT_DURATION;
            int priority = -1;
            
            sscanf(commandBuffer, "%s", cmd);
            
            // Command branches
            if (strcmp(cmd, "start") == 0) {
                isPaused = false;
                Serial.println(F("Scheduler started."));
            } 
            else if (strcmp(cmd, "stop") == 0) {
                isPaused = true;
                Serial.println(F("Scheduler stopped."));
            }
            else if (strcmp(cmd, "exec") == 0) {
                if (isSchedulerRunning()) {
                    char* ptr = commandBuffer + 5;
                    sscanf(ptr, "%s", taskName);
                    char* param = strtok(ptr, " ");
                    while ((param = strtok(NULL, " ")) != NULL) {
                        if (strcmp(param, "-t") == 0) {
                            duration = atol(strtok(NULL, " "));
                        } else if (strcmp(param, "-p") == 0) {
                            priority = atoi(strtok(NULL, " "));
                        }
                    }
                    if (priority == -1) {
                        priority = 10;
                    }
                    scheduler_add_task(taskName, duration, priority);
                } else {
                    Serial.println(F("Scheduler is stopped. Use 'start' to run the scheduler."));
                }
            } 
            else if (strcmp(cmd, "halt") == 0) {
                if (isSchedulerRunning()) {
                    sscanf(commandBuffer + 5, "%s", taskName);
                    scheduler_remove_task(taskName);
                } else {
                    Serial.println(F("Scheduler is stopped. Use 'start' to run the scheduler."));
                }
            } 

            else if (strcmp(cmd, "BTDIAG") == 0) {
              if (isSchedulerRunning()) {
                Serial.println(F("Stop the scheduler before Bluetooth diagnostics."));
              } else {
                bt_diagnostic();
              }
            }

            else if (strcmp(cmd, "inspect") == 0) {
                scheduler_inspect();
            }
            else if (strcmp(cmd, "CREATE") == 0) {
                if (!isSchedulerRunning()) {
                    char filename[20];
                    if (sscanf(commandBuffer + 7, "%19s", filename) == 1) {
                        createFile(filename);
                    }
                } else {
                    Serial.println(F("Scheduler is running. Use 'stop' before file operations."));
                }
            }
            else if (strcmp(cmd, "DELETE") == 0) {
                if (!isSchedulerRunning()) {
                    char filename[20];
                    if (sscanf(commandBuffer + 7, "%19s", filename) == 1) {
                        deleteFile(filename);
                    }
                } else {
                    Serial.println(F("Scheduler is running. Use 'stop' before file operations."));
                }
            }
            else if (strcmp(cmd, "VIEW") == 0) {
                listFiles();
            }
            // New Bluetooth file transfer commands
            else if (strcmp(cmd, "BTGET") == 0) {
                if (isSchedulerRunning()) {
                    Serial.println(F("Stop the scheduler before Bluetooth file operations."));
                } else {
                    bt_receive_file();
                }
            }
            else if (strcmp(cmd, "BTSEND") == 0) {
                if (isSchedulerRunning()) {
                    Serial.println(F("Stop the scheduler before Bluetooth file operations."));
                } else {
                    char filename[20];
                    if (sscanf(commandBuffer + 7, "%19s", filename) == 1) {
                        bt_send_file(filename);
                    } else {
                        Serial.println(F("Please specify a filename to send."));
                    }
                }
            }
            else {
                Serial.println(F("Invalid command!"));
            }
            memset(commandBuffer, 0, CMD_BUFFER_SIZE);
        } else {
            commandBuffer[bufferIndex++] = c;
        }
    }
}

void swap_out_task(int index) {
    unsigned int address = 0x0000 + (index * sizeof(ScheduledTask));
    byte* taskData = (byte*)&taskList[index];
    for (int i = 0; i < sizeof(ScheduledTask); i++) {
        writeEEPROM(address + i, taskData[i]);
        delay(5);  // ensure write completion
    }
    taskList[index].swapped = true;
    taskList[index].active = false;
    Serial.print(F("Swapped out task: "));
    Serial.println(taskList[index].name);
}

void swap_in_task(int index) {
    unsigned int address = 0x0000 + (index * sizeof(ScheduledTask));
    byte* taskData = (byte*)&taskList[index];
    for (int i = 0; i < sizeof(ScheduledTask); i++) {
        taskData[i] = readEEPROM(address + i);
    }
    taskList[index].swapped = false;
    taskList[index].active = true;
    Serial.print(F("Swapped in task: "));
    Serial.println(taskList[index].name);
}
