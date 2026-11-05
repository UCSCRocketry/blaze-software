/**
 * @file magnetometer.h
 * @brief Wrapper class for LIS2MDL magnetometer using SPI communication
 * 
 * This class is a simple interface for the LIS2MDL magnetometer, using 
 */

#pragma once

#include <Adafruit_LIS2MDL.h>

struct MagnetometerReading{
    float x;
    float y;
    float z;
};

/**
 * @class Magnetometer
 * @brief Wrapper class for the LIS2MDL sensor SPI communication
 */
class Magnetometer{
public:
    /**
     * @brief Constructor for the Magnetometer class
     * @param csPin Chip select pin used for SPI communication
     */
    explicit Magnetometer(uint8_t csPin);

    /**
     * @enum Status
     * @brief Status codes returned by Magnetometer methods
     */
    enum Status : int8_t {
        OK,
        ERR_NOT_INITIALIZED,
        ERR_SENSOR_INIT,
        ERR_NOT_IMPLEMENTED,
        ERR_SENSOR_READ
    };

    /**
     * @brief Starts the sensor
     * @param rate The desired update rate setting for the sensor, defaults to 50 Hz
     */
    int8_t start_up(lis2mdl_rate_t rate = LIS2MDL_RATE_50_HZ);
    int8_t self_test();
    int8_t set_rate(lis2mdl_rate_t rate);
    // int8_t get_id();
    int8_t reset();
    int8_t read(MagnetometerReading& reading);

private:
    Adafruit_LIS2MDL _sensor;
    uint8_t _csPin;
    bool _initialized;
};