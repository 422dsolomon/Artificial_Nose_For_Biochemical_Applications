/**
 * Collection script:
 *   https://github.com/edgeimpulse/example-data-collection-csv/blob/main/serial-data-collect-csv.py
 * 
 * Based on the work by Benjamin Cabé:
 *   https://github.com/kartben/artificial-nose
 * 
 * Sensors:
 *  MQ Sensor Suite
 *  
 * Author: Danny Solomon
 * Based on code from: Shawn Hymel
 * 
 * Date: Nov 23, 2022
 * License: 0BSD (https://opensource.org/licenses/0BSD)
 */

#include <Wire.h>

// Settings
#define SAMPLING_FREQ_HZ    4                         // Sampling frequency (Hz)
#define SAMPLING_PERIOD_MS  1000 / SAMPLING_FREQ_HZ   // Sampling period (ms)
#define NUM_SAMPLES         8                         // 8 samples at 4 Hz is 2 seconds

int sensor0 = A0;
int sensor1 = A1;
int sensor2 = A2;
int sensor3 = A3;
int sensor4 = A4;
int sensor5 = A5;

void setup() {
  // Start serial
  Serial.begin(115200);

  // Initialize sensor0
  while (!sensor0) {
    Serial.println("Trying to initialize sensor0...");
    delay(1000);
  }

  // Initialize sensor1
  while (!sensor1) {
    Serial.println("Trying to initialize sensor1...");
    delay(1000);
  }

  // Initialize sensor2
  while (!sensor2) {
    Serial.println("Trying to initialize sensor2...");
    delay(1000);
  }

   // Initialize sensor3
  while (!sensor3) {
    Serial.println("Trying to initialize sensor3...");
    delay(1000);
  }

   // Initialize sensor4
  while (!sensor4) {
    Serial.println("Trying to initialize sensor4...");
    delay(1000);
  }

   // Initialize sensor5
  while (!sensor5) {
    Serial.println("Trying to initialize sensor5...");
    delay(1000);
  }
}

void loop() {

  unsigned long timestamp;

  // Print header
  Serial.println("timestamp,sensor0,sensor1,sensor2,sensor3,sensor4,sensor5");

  // Transmit samples over serial port
  for (int i = 0; i < NUM_SAMPLES; i++) {

    // Take timestamp so we can hit our target frequency
    timestamp = millis();

    // Read from gas sensors (multichannel gas)
    unsigned int sensorValue0 = analogRead(sensor0);
    unsigned int sensorValue1 = analogRead(sensor1);
    unsigned int sensorValue2 = analogRead(sensor2);
    unsigned int sensorValue3 = analogRead(sensor3);
    unsigned int sensorValue4 = analogRead(sensor4);
    unsigned int sensorValue5 = analogRead(sensor5);

    // Print CSV data with timestamp
    Serial.print(timestamp);
    Serial.print(",");
    Serial.print(sensorValue0);
    Serial.print(",");
    Serial.print(sensorValue1);
    Serial.print(",");
    Serial.print(sensorValue2);
    Serial.print(",");
    Serial.print(sensorValue3);
    Serial.print(",");
    Serial.print(sensorValue4);
    Serial.print(",");
    Serial.print(sensorValue5);
    Serial.println();

    // Wait just long enough for our sampling period
    while (millis() < timestamp + SAMPLING_PERIOD_MS);
  }

  // Print empty line to transmit termination of recording
  Serial.println();
}
