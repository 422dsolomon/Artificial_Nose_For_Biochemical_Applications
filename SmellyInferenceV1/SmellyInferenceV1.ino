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
 * Date: Nov 25, 2022
 * License: 0BSD (https://opensource.org/licenses/0BSD)
 */

#include <Wire.h>

#include "artificial-nose_inferencing.h"                      // Name of Edge Impulse library

// Settings
#define DEBUG               1                         // 1 to print out debugging info
#define DEBUG_NN            false                     // Print out EI debugging info
#define ANOMALY_THRESHOLD   0.3                       // Scores above this are an "anomaly"
#define SAMPLING_FREQ_HZ    4                         // Sampling frequency (Hz)
#define SAMPLING_PERIOD_MS  1000 / SAMPLING_FREQ_HZ   // Sampling period (ms)
#define NUM_SAMPLES         EI_CLASSIFIER_RAW_SAMPLE_COUNT  // 4 samples at 4 Hz
#define READINGS_PER_SAMPLE EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME // 8

// Constants
#define BME680_I2C_ADDR     uint8_t(0x76)             // I2C address of BME680
#define PA_IN_KPA           1000.0                    // Convert Pa to KPa

// Preprocessing constants (drop the timestamp column)
float mins[] = {
  192.0, 115.0, 198.0, 61.0, 55.0
};
float ranges[] = {
  60.0, 87.0, 60.0, 65.0, 77.0
};
int sensor0 = A0;
int sensor1 = A1;
int sensor2 = A2;
int sensor3 = A3;
int sensor4 = A4;
int sensor5 = A5;

void setup() {
  // Start serial
  Serial.begin(115200);

  // Configure LCD
  tft.begin();
  tft.setRotation(3);
  tft.setFreeFont(&FreeSansBoldOblique24pt7b);
  tft.fillScreen(TFT_BLACK);

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
  static float raw_buf[NUM_SAMPLES * READINGS_PER_SAMPLE];
  static signal_t signal;
  float temp;
  int max_idx = 0;
  float max_val = 0.0;
  char str_buf[40];
  
  // Collect samples and perform inference
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
  
    // Read BME680 environmental sensor
    if (bme680.read_sensor_data()) {
      Serial.println("Error: Could not read from BME680");
      return;
    }
  
    // Read SGP30 sensor
    sgp_err = sgp_measure_iaq_blocking_read(&sgp_tvoc, &sgp_co2);
    if (sgp_err != STATUS_OK) {
      Serial.println("Error: Could not read from SGP30");
      return;
    }

    // Store raw data into the buffer
    raw_buf[(i * READINGS_PER_SAMPLE) + 0] = sensorValue0;
    raw_buf[(i * READINGS_PER_SAMPLE) + 1] = sensorValue1;
    raw_buf[(i * READINGS_PER_SAMPLE) + 2] = sensorValue2;
    raw_buf[(i * READINGS_PER_SAMPLE) + 3] = sensorValue3;
    raw_buf[(i * READINGS_PER_SAMPLE) + 4] = sensorValue4;
    raw_buf[(i * READINGS_PER_SAMPLE) + 5] = sensorValue5;

    // Perform preprocessing step (normalization) on all readings in the sample
    for (int j = 0; j < READINGS_PER_SAMPLE; j++) {
      temp = raw_buf[(i * READINGS_PER_SAMPLE) + j] - mins[j];
      raw_buf[(i * READINGS_PER_SAMPLE) + j] = temp / ranges[j];
    }

    // Wait just long enough for our sampling period
    while (millis() < timestamp + SAMPLING_PERIOD_MS);
  }

  // Print out our preprocessed, raw buffer
#if DEBUG
  for (int i = 0; i < NUM_SAMPLES * READINGS_PER_SAMPLE; i++) {
    Serial.print(raw_buf[i]);
    if (i < (NUM_SAMPLES * READINGS_PER_SAMPLE) - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();
#endif

  // Turn the raw buffer in a signal which we can the classify
  int err = numpy::signal_from_buffer(raw_buf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
      ei_printf("ERROR: Failed to create signal from buffer (%d)\r\n", err);
      return;
  }

  // Run inference
  ei_impulse_result_t result = {0};
  err = run_classifier(&signal, &result, DEBUG_NN);
  if (err != EI_IMPULSE_OK) {
      ei_printf("ERROR: Failed to run classifier (%d)\r\n", err);
      return;
  }

  // Print the predictions
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)\r\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    ei_printf("\t%s: %.3f\r\n", 
              result.classification[i].label, 
              result.classification[i].value);
  }

  // Print anomaly detection score
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("\tanomaly acore: %.3f\r\n", result.anomaly);
#endif

  // Find maximum prediction
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > max_val) {
      max_val = result.classification[i].value;
      max_idx = i;
    }
  }

  // Print predicted label and value to LCD if not anomalous
  tft.fillScreen(TFT_BLACK);
  if (result.anomaly < ANOMALY_THRESHOLD) {
    tft.drawString(result.classification[max_idx].label, 20, 60);
    sprintf(str_buf, "%.3f", max_val);
    tft.drawString(str_buf, 60, 120);
  } else {
    tft.drawString("Unknown", 20, 60);
    sprintf(str_buf, "%.3f", result.anomaly);
    tft.drawString(str_buf, 60, 120);
  }
}
