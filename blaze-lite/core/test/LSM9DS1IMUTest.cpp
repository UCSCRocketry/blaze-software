#include <Arduino.h>
#include <SPI.h>

#include "LSM9DS1IMU.h"

#define LSM9DS1_XGCS_PIN PA1 //accel/gyro
#define LSM9DS1_MCS_PIN PA2 //magnetometer

#define SPI_SCK_PIN  PA5
#define SPI_MISO_PIN PA6
#define SPI_MOSI_PIN PA7

LSM9DS1IMU imu(LSM9DS1_XGCS_PIN, LSM9DS1_MCS_PIN);

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
    delay(1000); // Allow time for serial monitor to connect

    Serial.println("Starting SPI");
    SPI.setSCLK(SPI_SCK_PIN);
    SPI.setMISO(SPI_MISO_PIN);
    SPI.setMOSI(SPI_MOSI_PIN);

    SPI.begin();
    delay(100); // Allow time for SPI to initialize
    Serial.println("Setting up IMU");
    imu.setUp();
}

void loop() {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    float tempC;

    imu.pollSensors();

    if (imu.readAccel(ax, ay, az)) {
        Serial.print("ACC (m/s^2): ");
        Serial.print(ax, 4);
        Serial.print(", ");
        Serial.print(ay, 4);
        Serial.print(", ");
        Serial.println(az, 4);
    } else {
        Serial.println("ACC: read failed");
    }

    if (imu.readGyro(gx, gy, gz)) {
        Serial.print("GYRO (dps): ");
        Serial.print(gx, 4);
        Serial.print(", ");
        Serial.print(gy, 4);
        Serial.print(", ");
        Serial.println(gz, 4);
    } else {
        Serial.println("GYRO: read failed");
    }

    if (imu.readMag(mx, my, mz)) {
        Serial.print("MAG (gauss): ");
        Serial.print(mx, 4);
        Serial.print(", ");
        Serial.print(my, 4);
        Serial.print(", ");
        Serial.println(mz, 4);
    } else {
        Serial.println("MAG: read failed");
    }

    if (imu.readTemp(tempC)) {
        Serial.print("TEMP (C): ");
        Serial.println(tempC, 2);
    } else {
        Serial.println("TEMP: read failed");
    }

    Serial.println("---");
    delay(200);
}
