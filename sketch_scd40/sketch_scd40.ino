#include <M5CoreS3.h>

#include "scd40.h"

// Consts
const uint16_t kBackgroundColor = BLACK;
const uint16_t kTextColor = LIGHTGREY;
const uint16_t kTextSize = 3;

const int kDisplayOffsetX = 4;
const int kDisplayOffsetY = 70;
const int kDisplayPositionInterval = 30;

const float kThresholdCO2 = 1000.0f;
const float kThresholdAHLow = 7.0f;
const float kThresholdAHMid = 11.0f;

SCD40 scd40_;

void setup() {
  M5.begin(true, true, false);

  // Initialize LCD
  M5.Lcd.fillScreen(kBackgroundColor);
  M5.Lcd.setTextColor(kTextColor, kBackgroundColor);
  M5.Lcd.setTextSize(kTextSize);
  scd40_.Initialize();
}

void loop() {
  SCD40::SensingInformation si;
  scd40_.GetSensingInformation(si);
  Display(si);
  scd40_.Wait();
}

void Display(const SCD40::SensingInformation &si) {
  M5.Lcd.setTextColor(kTextColor, kBackgroundColor);
  int x = kDisplayOffsetX;
  int y = kDisplayOffsetY;
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("TEMP[C]  : %5.1f\n", si.temperature);

  if (si.co2 > kThresholdCO2) {
    M5.Lcd.setTextColor(kTextColor, RED);
  } else {
    M5.Lcd.setTextColor(kTextColor, kBackgroundColor);
  }
  y += kDisplayPositionInterval;
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("CO2[ppm] : %5d\n", si.co2);

  if (si.absolute_humidity < kThresholdAHLow) {
    M5.Lcd.setTextColor(kTextColor, RED);
  } else  if (si.absolute_humidity < kThresholdAHMid) {
    M5.Lcd.setTextColor(BLACK, YELLOW);
  } else {
    M5.Lcd.setTextColor(kTextColor, kBackgroundColor);
  }
  y += kDisplayPositionInterval;
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("RH[%%]    : %5.1f\n", si.relative_humidity * 100);

  y += kDisplayPositionInterval;
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("AH[g/m^3]: %5.1f\n", si.absolute_humidity);
}

