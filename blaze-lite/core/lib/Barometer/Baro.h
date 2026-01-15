#pragma once

#include <Arduino.h>

#include "driver_ms5611_interface.h"

class Baro {
    public:
        //constructors/destructors
        Baro(uint8_t csPin);
        ~Baro();

        bool startUp();

        bool read();

        float getTemperature();     // Celsius
        float getPressureMbar();    // mbar
        float getPressurePa();      // Pascal
        float getAltitudeM(float seaLevelPressureMbar = 1013.25f);

    private:
        ms5611_handle_t handle;
        uint8_t csPin;
        float temperatureC;
        float pressureMbar;
};