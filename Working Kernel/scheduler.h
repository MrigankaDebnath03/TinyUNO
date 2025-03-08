// #ifndef SCHEDULER_H
// #define SCHEDULER_H

// #include <Arduino.h>

// // Maximum number of tasks allowed simultaneously in RAM
// #define MAX_TASKS 3
// // Maximum tasks that can be registered overall (active + swapped)
// #define MAX_REGISTERED_TASKS 5
// #define CMD_BUFFER_SIZE 50
// #define DEFAULT_DURATION 3000

// typedef void (*TaskFunction)();

// struct ScheduledTask {
//     char name[10];           // Reduced from 20 to 10
//     TaskFunction function;
//     unsigned long duration;
//     unsigned long startTime;
//     unsigned long endTime;
//     int priority;
//     bool active;
//     bool swapped;
// };

// extern ScheduledTask taskList[MAX_REGISTERED_TASKS];
// extern int taskCount;
// extern int activeTaskCount;
// extern bool isPaused;
// extern char commandBuffer[CMD_BUFFER_SIZE];

// void scheduler_init();
// void scheduler_register_task(const char* name, TaskFunction function);
// void scheduler_add_task(const char* name, unsigned long duration, int priority);
// void scheduler_remove_task(const char* name);
// void scheduler_run();
// void scheduler_inspect();
// void scheduler_handle_command();
// void swap_out_task(int index);
// void swap_in_task(int index);
// bool isSchedulerRunning(); // New function to check if the scheduler is running

// #endif

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// Maximum number of tasks allowed simultaneously in RAM
#define MAX_TASKS 3
// Maximum tasks that can be registered overall (active + swapped)
#define MAX_REGISTERED_TASKS 5
#define CMD_BUFFER_SIZE 50
#define DEFAULT_DURATION 3000

typedef void (*TaskFunction)();

struct ScheduledTask {
    char name[10];           // Reduced from 20 to 10
    TaskFunction function;
    unsigned long duration;
    unsigned long startTime;
    unsigned long endTime;
    int priority;
    bool active;
    bool swapped;
};

extern ScheduledTask taskList[MAX_REGISTERED_TASKS];
extern int taskCount;
extern int activeTaskCount;
extern bool isPaused;
extern char commandBuffer[CMD_BUFFER_SIZE];

void scheduler_init();
void scheduler_register_task(const char* name, TaskFunction function);
void scheduler_add_task(const char* name, unsigned long duration, int priority);
void scheduler_remove_task(const char* name);
void scheduler_run();
void scheduler_inspect();
void scheduler_handle_command();
void swap_out_task(int index);
void swap_in_task(int index);
bool isSchedulerRunning(); // New function to check if the scheduler is running

#endif
