#include <Arduino.h>
#include <Wire.h>
#include "MS5611.h"

//-------INTIALIZATION-------//

MS5611 baro(0x77);
// Running altitude marker (prints every X feet)
const float FEET_MARKER_INTERVAL = 100.0;  
float lastMarker = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {}   // Wait for USB serial on ESP32
    
    Serial.println("Initializing MS5611 Barometer...");
    Wire.begin();

    // Start MS5611 sensor
    if (baro.begin()) {
        Serial.print("MS5611 detected at address: ");
        Serial.println(baro.getAddress());
    } else {
        Serial.println("ERROR: MS5611 not detected!");
        Serial.println("Check wiring + I2C connections.");
        while (1);  // Stop here
    }

    Serial.println("Barometer ready.\n");
}

//--------MAIN LOOP--------//

void loop() {
    // Read sensor (Rob Tillaart library)
    baro.read();
    
    // Extract values
    float tempC = baro.getTemperature();
    float pressure_mbar = baro.getPressure();
    float alt_meters = baro.getAltitude();

    // Convert altitude to feet (1 m = 3.28084 ft)
    float alt_feet = alt_meters * 3.28084;

    // display values
    Serial.print("Temp (C): ");
    Serial.print(tempC, 2);

    Serial.print(" | Pressure (mBar): ");
    Serial.print(pressure_mbar, 2);

    Serial.print(" | Altitude (m): ");
    Serial.print(alt_meters, 2);

    Serial.print(" | Altitude (ft): ");
    Serial.println(alt_feet, 2);

    // markers
    if (alt_feet - lastMarker >= FEET_MARKER_INTERVAL) {
        Serial.print("---- Passed ");
        Serial.print(lastMarker + FEET_MARKER_INTERVAL);
        Serial.println(" ft ----");

        lastMarker += FEET_MARKER_INTERVAL;
    }

    delay(100);  // .1s?
}



