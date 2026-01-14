#include <Arduino.h>
#include <SPI.h>

#include "Baro.h"

static constexpr uint8_t BARO_CS_PIN = PB9;

Baro baro(BARO_CS_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial){
        delay(50);
    }
  delay(5000);

  Serial.println();
  Serial.println("Baro (MS5611_SPI) tester");

  SPI.begin();
  const bool ok = baro.startUp();
  Serial.print("begin(): ");
  Serial.println(ok ? "OK" : "FAIL");
  Serial.print("deviceID: 0x");
  Serial.println(baro.getDeviceID(), HEX);

  // Valid OSR values are 8..12 (OSR_ULTRA_LOW .. OSR_ULTRA_HIGH)
  baro.setOverSamplingRate((uint8_t)OSR_STANDARD);

  Serial.println("Temp_C\tPressure_mBar\tAltitude_m");
}

void loop() {
  const int rv = baro.read();
  if (rv != MS5611_READ_OK)
  {
    Serial.print("read() error: ");
    Serial.print(rv);
    delay(500);
    return;
  }

  const float tempC = baro.getTemperature();
  const float pressureMbar = baro.getPressure();
  const float altitudeM = baro.getBaroAltitude(baro.getSeaLevelPressure());

  Serial.print(tempC, 2);
  Serial.print('\t');
  Serial.print(pressureMbar, 2);
  Serial.print('\t');
  Serial.println(altitudeM, 2);

  delay(500);
}