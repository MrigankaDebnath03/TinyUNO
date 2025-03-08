#include "distance_task.h"
#include <Arduino.h>
#include <SD.h>

const int trigPin = 9;
const int echoPin = 8;

void setup_distance_sensor() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

void distance_task_wrapper() {
    // Measure distance
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    int distance = duration * 0.034 / 2;

    // Log to Serial
    Serial.print(F("Distance: "));
    Serial.print(distance);
    Serial.println(F(" cm"));

    // Log to SD Card
    File dataFile = SD.open("distance_log.txt", FILE_WRITE);
    if (dataFile) {
        dataFile.print(millis());
        dataFile.print(F(","));
        dataFile.println(distance);
        dataFile.close();
    }
}