# Arduino Scheduler (Temporary Commit)

This is a temporary commit for the Arduino Scheduler project. The scheduler is designed to manage multiple tasks on an Arduino using function pointers and supports dynamic task addition, removal, and prioritization.

## Features
- **Task Scheduling**: Add, remove, and manage tasks dynamically.
- **Priority Support**: Tasks can be assigned priorities (higher number = higher priority).
- **Serial Commands**: Control the scheduler via the Serial Monitor.
- **Non-blocking Execution**: Tasks use `millis()` for timing, avoiding `delay()`.

## File Structure
Scheduler/
├── Scheduler.ino (Main program)
├── scheduler.h (Scheduler header)
├── scheduler.cpp (Scheduler implementation)
├── led_task.h (LED task header)
├── led_task.cpp (LED task implementation)
├── distance_task.h (Distance sensor header)
└── distance_task.cpp (Distance sensor implementation)


## Usage
1. Open `Scheduler.ino` in the Arduino IDE.
2. Upload the code to your Arduino.
3. Use the Serial Monitor (9600 baud) to send commands:
   - `exec <taskname> [-p <priority>] [-t <interval>]`: Add a task.
   - `halt <taskname>`: Remove a task.
   - `inspect`: List all tasks.
   - `cont`: Resume execution after inspection.

## Example Commands
```bash
exec led -t 1000          # Blink LED every 1 second
exec distance -t 500      # Measure distance every 500ms
inspect                   # List all tasks
halt led                  # Stop the LED task
