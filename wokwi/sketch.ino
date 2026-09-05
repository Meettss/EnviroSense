#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int POT_TEMP = 33;
const int POT_HUMIDITY = 34;
const int POT_GAS = 35;
const int LED_PIN = 2;

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

  Serial.print("Temp raw: ");
  Serial.print(tempRaw);
  Serial.print(" | Humidity raw: ");
  Serial.print(humidityRaw);
  Serial.print(" | Gas raw: ");
  Serial.println(gasRaw);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("EnviroSense Live");
  display.print("Temp: "); display.println(tempRaw);
  display.print("Humid: "); display.println(humidityRaw);
  display.print("Gas: "); display.println(gasRaw);
  display.display();

  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}