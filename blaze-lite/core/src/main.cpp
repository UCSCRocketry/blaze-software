#include <Arduino.h>
#include <SPI.h>

#include "Baro.h"

static constexpr uint8_t BARO_CS_PIN = PB9;

Baro baro(BARO_CS_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    while (!Serial){
        delay(50);
    }

  Serial.println();
  Serial.println("Baro (MS5611_SPI) tester");

  
  const bool ok = baro.startUp();
  delay(5000);
  Serial.print("begin(): ");
  Serial.println(ok ? "OK" : "FAIL");
  Serial.print("deviceID: 0x");
  Serial.println(baro.getDeviceID(), HEX);
  baro.setPressureOffset(-1013);

    Serial.print("PROM: ");
    for (uint8_t i = 0; i < 7; i++)
    {
        if (i) Serial.print(' ');
        Serial.print("0x");
        Serial.print(baro.getProm(i), HEX);
    }
    Serial.print("  CRC=0x");
    Serial.println(baro.getCRC(), HEX);

  // Valid OSR values are 8..12 (OSR_ULTRA_LOW .. OSR_ULTRA_HIGH)
  baro.setOverSamplingRate(OSR_ULTRA_LOW);

  Serial.println("Temp_C\tPressure_mBar\tAltitude_m");
}

void loop() {
    const int rv = baro.read();
        if (rv != MS5611_READ_OK) {
        Serial.print("read() error: ");
        Serial.print(rv);
        delay(500);
        return;
    }
    const float tempC = baro.getTemperature();
    const float pressurePascal = baro.getPressurePascal();
    const float altitudeM = baro.getBaroAltitude(baro.getSeaLevelPressure());

    Serial.print(tempC, 2);
    Serial.print('\t');
    Serial.print(pressurePascal, 2);
    Serial.print('\t');
    Serial.println(altitudeM, 2);
    delay(500);
}