#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "reg_model.h"
#include "clf_model.h"
#include "reg_model_v2.h"
#include "clf_model_v2.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int POT_TEMP = 33;
const int POT_HUMIDITY = 34;
const int POT_GAS = 35;
const int LED_PIN = 2;
bool useV2Model = false;  // set to false to use V1, true to use V2 

// ReLU activation: negative numbers become 0, positive numbers stay the same
float relu(float x) {
  if (x > 0) {
    return x;
  } else {
    return 0;
  }
}

// Runs a single input through a 3-8-1 model and returns the raw output
float runModel(float input[3], 
               const float w0[3][8], const float b0[8],
               const float w1[8][1], const float b1[1]) {
  
  float hidden[8];

  // Step 1: calculate each of the 8 hidden neurons
  for (int h = 0; h < 8; h++) {
    float sum = 0;
    for (int i = 0; i < 3; i++) {
      sum += input[i] * w0[i][h];
    }
    sum += b0[h];
    hidden[h] = relu(sum);
  }

  // Step 2: combine hidden neurons into the final output
  float output = 0;
  for (int h = 0; h < 8; h++) {
    output += hidden[h] * w1[h][0];
  }
  output += b1[0];

  return output;
} 
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true); // stop here if display isn't detected
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("EnviroSense Ready");
  display.display();
}

void loop() {
  int tempRaw = analogRead(POT_TEMP);
  int humidityRaw = analogRead(POT_HUMIDITY);
  int gasRaw = analogRead(POT_GAS);

  // Scale raw ADC values (0-4095) into realistic sensor ranges
  float temperature = map(tempRaw, 0, 4095, 15, 60);
  float humidity = map(humidityRaw, 0, 4095, 0, 100);
  float gas = map(gasRaw, 0, 4095, 75, 560);

  float inputs[3] = {temperature, humidity, gas};

  // Run both models
float riskIndex, anomalyScore;

if (useV2Model) {
  riskIndex = runModel(inputs, reg_model_v2_w0, reg_model_v2_b0, reg_model_v2_w1, reg_model_v2_b1);
  anomalyScore = runModel(inputs, clf_model_v2_w0, clf_model_v2_b0, clf_model_v2_w1, clf_model_v2_b1);
} else {
  riskIndex = runModel(inputs, reg_model_w0, reg_model_b0, reg_model_w1, reg_model_b1);
  anomalyScore = runModel(inputs, clf_model_w0, clf_model_b0, clf_model_w1, clf_model_b1);
}
  bool isAnomaly = anomalyScore > 0.5;

  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" | Humidity: "); Serial.print(humidity);
  Serial.print(" | Gas: "); Serial.print(gas);
  Serial.print(" | Risk: "); Serial.print(riskIndex);
  Serial.print(" | Anomaly: "); Serial.println(isAnomaly ? "YES" : "NO");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("EnviroSense Live");
  display.print("Risk: "); display.println(riskIndex);
  display.print("Status: "); display.println(isAnomaly ? "ANOMALY!" : "Normal");
  display.display();

  digitalWrite(LED_PIN, isAnomaly ? HIGH : LOW);
  delay(500);
}