#include <Arduino.h>
#include <M5CoreS3.h>
#include <Wire.h>

#include <cmath>

// Consts
const int16_t kScdAddress = 0x62;
const uint32_t kInitialTimeMilliSec = 1000;
const uint32_t kIntervalTimeMilliSec = 5000;
const uint16_t kBackgroundColor = M5.Lcd.color565(61, 61, 61);
const uint16_t kTextColor = M5.Lcd.color565(224, 224, 224);
const uint16_t kTextSize = 2;

void setup() {
  M5.begin(true, true, false);

  // Initialize LCD
  M5.Lcd.fillScreen(kBackgroundColor);
  M5.Lcd.setTextColor(kTextColor, kBackgroundColor);
  M5.Lcd.setTextSize(kTextSize);

  // Initialize I2C
  M5.Axp.setBoostBusOutEn(true);
  Wire.begin(2, 1);

  // wait until sensors are ready, > 1000 ms according to datasheet
  delay(kInitialTimeMilliSec);

  // start scd measurement in periodic mode, will update every 5 s
  Wire.beginTransmission(kScdAddress);
  Wire.write(0x21);
  Wire.write(0xb1);
  Wire.endTransmission();

  // wait for first measurement to be finished
  delay(kIntervalTimeMilliSec);
}

void loop() {
  // send read data command
  Wire.beginTransmission(kScdAddress);
  Wire.write(0xec);
  Wire.write(0x05);
  Wire.endTransmission();

  // read measurement data: 2 bytes co2, 1 byte CRC,
  // 2 bytes T, 1 byte CRC, 2 bytes RH, 1 byte CRC,
  // 2 bytes sensor status, 1 byte CRC
  // stop reading after 12 bytes (not used)
  // other data like  ASC not included
  const auto kDataNum = 12;
  Wire.requestFrom(kScdAddress, kDataNum);
  auto counter = 0;
  uint8_t data[kDataNum];
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }

  // floating point conversion according to datasheet
  auto co2 = (float)((uint16_t)data[0] << 8 | data[1]);
  // convert T in degC
  auto temperature =
      -45 + 175 * (float)((uint16_t)data[3] << 8 | data[4]) / 65536;
  // convert RH in %
  auto relative_humidity = (float)((uint16_t)data[6] << 8 | data[7]) / 65536;

  auto e = 6.1078 * std::pow(10, (7.5 * temperature / (temperature + 237.3)));
  auto a = (217 * e) / (temperature + 273.15);
  auto absolute_humidity = a * relative_humidity;

  M5.Lcd.setCursor(10, 80);
  M5.Lcd.printf("CO2[ppm]       : %.1f\n", co2);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.printf("Temperature[C] : %.1f\n", temperature);
  M5.Lcd.setCursor(10, 120);
  M5.Lcd.printf("RH[%%]          : %.1f\n", relative_humidity * 100);
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.printf("AH[g/m^3]      : %.1f\n", absolute_humidity);

  delay(kIntervalTimeMilliSec);
}
