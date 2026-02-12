/**
 * @file KX134Accelerometer.cpp
 * @brief Implementation of KX134Accelerometer class
 */

#include "KX134Accelerometer.h"

KX134Accelerometer::KX134Accelerometer() : _initialized(false) {
}

KX134Accelerometer::~KX134Accelerometer() {
}

bool KX134Accelerometer::begin(uint8_t csPin) {
    // Initialize with default SPI settings
    if (_kx134.begin(csPin)) {
        _initialized = true;
        return true;
    }
    _initialized = false;
    return false;
}

bool KX134Accelerometer::begin(SPIClass &spiPort, SPISettings spiSettings, uint8_t csPin) {
    // Initialize with custom SPI settings
    if (_kx134.begin(spiPort, spiSettings, csPin)) {
        _initialized = true;
        return true;
    }
    _initialized = false;
    return false;
}

bool KX134Accelerometer::isReady() const {
    return _initialized;
}

bool KX134Accelerometer::reset() {
    if (!_initialized) {
        return false;
    }
    return _kx134.softwareReset();
}

bool KX134Accelerometer::enable(bool enable) {
    if (!_initialized) {
        return false;
    }
    return _kx134.enableAccel(enable);
}

bool KX134Accelerometer::setRange(uint8_t range) {
    if (!_initialized) {
        return false;
    }
    return _kx134.setRange(range);
}

bool KX134Accelerometer::enableDataEngine(bool enable) {
    if (!_initialized) {
        return false;
    }
    return _kx134.enableDataEngine(enable);
}

bool KX134Accelerometer::setOutputDataRate(uint8_t odr) {
    if (!_initialized) {
        return false;
    }
    return _kx134.setOutputDataRate(odr);
}

float KX134Accelerometer::getOutputDataRate() {
    if (!_initialized) {
        return 0.0f;
    }
    return _kx134.getOutputDataRate();
}

bool KX134Accelerometer::dataReady() {
    if (!_initialized) {
        return false;
    }
    return _kx134.dataReady();
}

bool KX134Accelerometer::getAccelData(outputData *data) {
    if (!_initialized || data == nullptr) {
        return false;
    }
    return _kx134.getAccelData(data);
}

bool KX134Accelerometer::getRawAccelData(rawOutputData *data) {
    if (!_initialized || data == nullptr) {
        return false;
    }
    return _kx134.getRawAccelData(data);
}

uint8_t KX134Accelerometer::getUniqueID() {
    if (!_initialized) {
        return 0;
    }
    return _kx134.getUniqueID();
}

int8_t KX134Accelerometer::getOperatingMode() {
    if (!_initialized) {
        return -1;
    }
    return _kx134.getOperatingMode();
}

