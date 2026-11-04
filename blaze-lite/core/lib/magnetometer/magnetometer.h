#pragma once

#include <Adafruit_LIS2MDL.h>

struct MagnetometerReading{
    float x;
    float y;
    float z;
};

class Magnetometer{
public:
    explicit Magnetometer(uint8_t csPin);

     enum Status : int8_t {
        OK,
        ERR_NOT_INITIALIZED,
        ERR_SENSOR_INIT,
        ERR_NOT_IMPLEMENTED,
        ERR_SENSOR_READ
    };

    int8_t self_test();
    int8_t start_up(lis2mdl_rate_t rate);
    int8_t set_rate(lis2mdl_rate_t rate);
    // int8_t get_id();
    int8_t reset();
    int8_t read(MagnetometerReading& reading);

private:
    Adafruit_LIS2MDL _sensor;
    uint8_t _csPin;
    bool _initialized;
};