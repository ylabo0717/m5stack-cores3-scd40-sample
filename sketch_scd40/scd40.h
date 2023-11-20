#include <Arduino.h>
#include <M5CoreS3.h>
#include <Wire.h>

class SCD40 {
 public:
  SCD40() {}
  ~SCD40() {}
  void Initialize() {
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
    Wait();
  }

  struct SensingInformation {
    float temperature;
    uint16_t co2;
    float relative_humidity;
    float absolute_humidity;
  };
  void GetSensingInformation(SensingInformation &si) {
    uint16_t word[3];
    GetDataFromDevice(word);
    si.co2 = word[0];
    si.temperature = CalcTemperature(word[1]);
    si.relative_humidity = CalcRelativeHumidity(word[2]);
    si.absolute_humidity =
        CalcAbsoluteHumidity(si.temperature, si.relative_humidity);
  }

  void Wait() { delay(kIntervalTimeMilliSec); }

 private:
  void GetDataFromDevice(uint16_t *word) {
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
    word[0] = Byte2Word(data[0], data[1]);
    word[1] = Byte2Word(data[3], data[4]);
    word[2] = Byte2Word(data[6], data[7]);
  }

  inline uint16_t Byte2Word(uint8_t msb, uint8_t lsb) {
    return (static_cast<uint16_t>(msb) << 8) | lsb;
  }

  float CalcTemperature(uint16_t data) {
    auto temperature = -45 + 175 * static_cast<float>(data) / 65536;
    return temperature;
  }

  float CalcRelativeHumidity(uint16_t data) {
    auto relative_humidity = static_cast<float>(data) / 65536;
    return relative_humidity;
  }

  float CalcAbsoluteHumidity(float temperature, float relative_humidity) {
    auto e = 6.1078 * std::pow(10, (7.5 * temperature / (temperature + 237.3)));
    auto a = (217 * e) / (temperature + 273.15);
    auto absolute_humidity = a * relative_humidity;
    return absolute_humidity;
  }

  // Consts.
  const int16_t kScdAddress = 0x62;
  const uint32_t kInitialTimeMilliSec = 1000;
  const uint32_t kIntervalTimeMilliSec = 5000;
};
