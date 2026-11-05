/**
 * @file magnetometer.cpp
 * @brief Implementation of the Magnetometer wrapper class
 */

#include "magnetometer.h"

Magnetometer::Magnetometer(uint8_t csPin)
    : _sensor(), _csPin(csPin), _initialized(false) {}

int8_t Magnetometer::start_up(lis2mdl_rate_t rate){
    if(!_sensor.begin_SPI(_csPin)) return ERR_NOT_INITIALIZED;

    _sensor.setDataRate(rate);
    _initialized = true;

    int8_t result = self_test();
    if(result != SUCCESS){
        _initialized = false;
        return result;
    }

    return SUCCESS;
}

int8_t Magnetometer::self_test(){
    if(!_initialized) return ERR_NOT_INITIALIZED;

    sensors_event_t event{};
    if(!_sensor.getEvent(&event)){
        return ERR_SENSOR_INIT;
    }

    return SUCCESS;
}

// int8_t Magnetometer::get_id(){
//     if(!_initialized) return ERR_NOT_INITIALIZED;

//     return _sensor.;
// }

int8_t Magnetometer::set_rate(lis2mdl_rate_t rate){
    if(!_initialized) return ERR_NOT_INITIALIZED;

    _sensor.setDataRate(rate);
    return SUCCESS;
}

int8_t Magnetometer::reset(){
    if(!_initialized) return ERR_NOT_INITIALIZED;

    // save and set rate becuase _sensor.reset() sets the rate to LIS2MDL_100_HZ
    lis2mdl_rate_t rate = _sensor.getDataRate();
    _sensor.reset();
    _sensor.setDataRate(rate);

    return SUCCESS;
}

int8_t Magnetometer::read(MagnetometerReading& reading){
    if(!_initialized) return ERR_NOT_INITIALIZED;

    sensors_event_t event{};
    if(!_sensor.getEvent(&event)){
        return ERR_SENSOR_READ;
    }

    reading = {event.magnetic.x, event.magnetic.y, event.magnetic.z};
    return SUCCESS;
}