/**
 * @file LSM9DS1IMU.h
 * @brief Wrapper class for Adafruit LSM9DS1 IMU using SPI communication
 * 
 * This class provides a simplified interface for interacting with the LSM9DS1
 * IMU using SPI communication. It wraps the Adafruit_LSM9DS1 class
 * from the Adafruit LSM9DS1 Library.
 */

#ifndef LSM9DS1IMU_H
#define LSM9DS1IMU_H

#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

class LSM9DS1IMU {
public:
    LSM9DS1IMU(uint8_t accelGyroCsPin, uint8_t magCsPin);
    ~LSM9DS1IMU();

    bool setUp();
    void pollSensors();
    bool readAccel(float &x, float &y, float &z);
    bool readGyro(float &x, float &y, float &z);
    bool readMag(float &x, float &y, float &z);
    bool readTemp(float &celsius);

private:
    Adafruit_LSM9DS1 _lsm9ds1;
    sensors_event_t _accel;
    sensors_event_t _gyro;
    sensors_event_t _mag;
    sensors_event_t _temp;
    bool _hasSample;
    bool _isReady;
};

#endif