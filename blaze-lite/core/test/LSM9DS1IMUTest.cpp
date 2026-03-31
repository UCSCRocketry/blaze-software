#include <Arduino.h>
#include <SPI.h>

#include "LSM9DS1IMU.h"

#define LSM9DS1_XGCS_PIN PB12
#define LSM9DS1_MCS_PIN PB13

LSM9DS1IMU imu(LSM9DS1_XGCS_PIN, LSM9DS1_MCS_PIN);

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }

    SPI.begin();

    pinMode(LSM9DS1_XGCS_PIN, OUTPUT);
    pinMode(LSM9DS1_MCS_PIN, OUTPUT);
    digitalWrite(LSM9DS1_XGCS_PIN, HIGH);
    digitalWrite(LSM9DS1_MCS_PIN, HIGH);

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
