/**
 * @file Baro.cpp
 * @brief Implementation of the Baro wrapper class
 */

#include "Baro.h"

Baro::Baro(uint8_t CS) : cs_pin(CS), baro(CS), _initialized(false) {
}

bool Baro::init() {
  if (baro.begin()) {
    _initialized = true;

    // Reset the sensor
    baro.reset();

    return true;
  }
  _initialized = false;
  return false;
}

bool Baro::isReady() const {
  return _initialized;
}

int Baro::read() {
  if (!_initialized) return -1;
  return baro.read();
}

float Baro::getPressure() {
  if (!_initialized) return 0.0f;
  return baro.getPressure();
}

float Baro::getTemperature() {
  if (!_initialized) return 0.0f;
  return baro.getTemperature();
}

uint8_t Baro::getDeviceID() {
  if (!_initialized) return 0;
  return baro.getDeviceID();
}

float Baro::getAltitude() {
  if (!_initialized) return 0.0f;
  return baro.getAltitude();
}
