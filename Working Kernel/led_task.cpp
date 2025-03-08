#include "led_task.h"
#include <Arduino.h>

void setup_led() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void led_task_wrapper() {
    static unsigned long lastToggle = 0;
    static bool state = LOW;
    unsigned long currentMillis = millis();

    if (currentMillis - lastToggle >= 100) {
        state = !state;
        digitalWrite(LED_BUILTIN, state);
        lastToggle = currentMillis;
    }
}