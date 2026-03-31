/**
 * @file LSM9DS1IMU.cpp
 * @brief Implementation of LSM9DS1IMU class
 */

#include <Arduino.h>

#include "LSM9DS1IMU.h"

LSM9DS1IMU::LSM9DS1IMU(uint8_t accelGyroCsPin, uint8_t magCsPin)
    : _lsm9ds1(accelGyroCsPin, magCsPin),
      _hasSample(false),
      _isReady(false) {}

LSM9DS1IMU::~LSM9DS1IMU() {}

void LSM9DS1IMU::setUp() {
    Serial.println("Initializing LSM9DS1 IMU...");
    while (!_lsm9ds1.begin()) {
        Serial.println("Failed to initialize LSM9DS1 IMU! Retrying...");
        delay(1000);
    }
    _isReady = true;
    Serial.println("Setting sensor range");
    _lsm9ds1.setupAccel(_lsm9ds1.LSM9DS1_ACCELRANGE_16G, _lsm9ds1.LSM9DS1_ACCELDATARATE_952HZ);
    _lsm9ds1.setupMag(_lsm9ds1.LSM9DS1_MAGGAIN_16GAUSS);
    _lsm9ds1.setupGyro(_lsm9ds1.LSM9DS1_GYROSCALE_245DPS);
}

void LSM9DS1IMU::pollSensors() {
    if (!_isReady) {
        _hasSample = false;
        return;
    }

    _lsm9ds1.read();
    _lsm9ds1.getEvent(&_accel, &_mag, &_gyro, &_temp);
    _hasSample = true;
}

bool LSM9DS1IMU::readAccel(float &x, float &y, float &z) {
    if (!_isReady) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }
    if (!_hasSample) {
        pollSensors();
    }
    if (!_hasSample) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }

    x = _accel.acceleration.x;
    y = _accel.acceleration.y;
    z = _accel.acceleration.z;
    return true;
}

bool LSM9DS1IMU::readGyro(float &x, float &y, float &z) {
    if (!_isReady) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }
    if (!_hasSample) {
        pollSensors();
    }
    if (!_hasSample) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }

    x = _gyro.gyro.x;
    y = _gyro.gyro.y;
    z = _gyro.gyro.z;
    return true;
}

bool LSM9DS1IMU::readMag(float &x, float &y, float &z) {
    if (!_isReady) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }
    if (!_hasSample) {
        pollSensors();
    }
    if (!_hasSample) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return false;
    }

    x = _mag.magnetic.x;
    y = _mag.magnetic.y;
    z = _mag.magnetic.z;
    return true;
}

bool LSM9DS1IMU::readTemp(float &celsius) {
    if (!_isReady) {
        celsius = 0.0f;
        return false;
    }
    if (!_hasSample) {
        pollSensors();
    }
    if (!_hasSample) {
        celsius = 0.0f;
        return false;
    }

    celsius = _temp.temperature;
    return true;
}