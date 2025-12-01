#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "barometer.h"
#include "telemetry_logger.h"

const long baudrate = 115200;
const int chipSelect = 3;
const int samplingInterval = 100;  // milliseconds (10 Hz sampling)

// Global objects
Barometer barometer;
TelemetryLogger telemetryLogger("flight_data.csv");

unsigned long lastSampleTime = 0;

void setup()
{
    Serial.begin(baudrate);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println("\n\n=== Rocket Telemetry System ===");
    Serial.println("Initializing SD Card...");

    if (!SD.begin(chipSelect)) {
        Serial.println("ERROR: SD Card failed, or not present");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("✓ SD Card initialized");

    // Initialize barometer
    Serial.println("Initializing BMP280 Barometer...");
    if (!barometer.init()) {
        Serial.println("ERROR: Barometer initialization failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("✓ BMP280 Barometer initialized");

    // Initialize telemetry logger
    Serial.println("Initializing Telemetry Logger...");
    if (!telemetryLogger.init()) {
        Serial.println("ERROR: Telemetry logger initialization failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("✓ Telemetry Logger initialized");
    
    Serial.println("\n=== System Ready - Logging Started ===\n");
    lastSampleTime = millis();
}

void loop()
{
    unsigned long currentTime = millis();
    
    // Sample barometer at regular intervals
    if (currentTime - lastSampleTime >= samplingInterval) {
        lastSampleTime = currentTime;
        
        // Read barometer
        BarometerReading reading = barometer.readSensor();
        
        // Log to SD card
        if (telemetryLogger.logReading(reading)) {
            // Print to serial for monitoring
            Serial.print("[");
            Serial.print(reading.timestamp);
            Serial.print("ms] P:");
            Serial.print(reading.pressure / 100.0, 1);  // Convert to hPa for display
            Serial.print("hPa | Alt:");
            Serial.print(reading.altitude, 1);
            Serial.print("m | Temp:");
            Serial.print(reading.temperature, 1);
            Serial.println("°C");
        } else {
            Serial.println("ERROR: Failed to log reading!");
        }
    }
}
