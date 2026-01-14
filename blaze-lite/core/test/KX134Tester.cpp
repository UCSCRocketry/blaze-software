// KX134Accelerometer SPI tester
// Continuously prints X/Y/Z acceleration (g) over Serial.

#include <Arduino.h>
#include <SPI.h>

#include "KX134Accelerometer.h"

// Chip-select pin for your KX134.
// Override at build time with `-D KX134_CS_PIN=PA4` (or whatever pin you wired).
#ifndef KX134_CS_PIN
#define KX134_CS_PIN PB14
#endif

// Conservative SPI settings to start with.
static SPISettings kx134SpiSettings(1000000, MSBFIRST, SPI_MODE0);

static KX134Accelerometer accel;


void setup() {
	Serial.begin(9600);
	uint32_t start = millis();
	while (!Serial) {
		delay(50);
	}
    delay(5000); //wait for serial monitor

	Serial.println();
	Serial.println(F("KX134Accelerometer SPI tester"));

    //Ardunio pin setup
	pinMode(KX134_CS_PIN, OUTPUT);
	digitalWrite(KX134_CS_PIN, HIGH);

	SPI.begin();

	if (!accel.begin(SPI, kx134SpiSettings, KX134_CS_PIN)) {
		Serial.println("Error: KX134 begin() failed");
        while (true) {
            delay(500);
        }   
	}

	// Basic recommended init sequence.
	accel.reset();
	delay(50);
	accel.enableDataEngine(true);
	accel.setRange(SFE_KX134_RANGE64G);
	accel.enable(true);

	Serial.print("KX134 ready. Unique ID: 0x");
	Serial.println(accel.getUniqueID(), HEX);
}

void loop() {
	static outputData data{};
	if (accel.dataReady()) {
		if (accel.getAccelData(&data)) {
			Serial.print(F("x="));
			Serial.print(data.xData, 6);
			Serial.print(F(" g\t"));

			Serial.print(F("y="));
			Serial.print(data.yData, 6);
			Serial.print(F(" g\t"));

			Serial.print(F("z="));
			Serial.print(data.zData, 6);
			Serial.println(F(" g"));
		}
		else {
			Serial.println(F("Error: getAccelData() failed"));
		}
	}
	delay(50);
}