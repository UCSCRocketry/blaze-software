#include "Baro.h"
#include "driver_ms5611_basic.h"

Baro::Baro(ms5611_interface_t interface, uint8_t addr_pin) 
: interface(interface), CS_PIN(addr_pin){
    ms5611_basic_init(interface, addr_pin);
}

int Baro::read() {
    return ms5611_basic_read(&temperature, &seaLevelPressure);
}

bool startUp() {
    return true;
}

uint32_t getDeviceID() {
    return 0;
}
uint16_t getProm(uint8_t index) {
    return 0;
}
uint16_t getCRC() {
    return 0;
}

//configuration functions
void setOverSamplingRate(uint8_t osr) {

}

void setPressureOffset(float offset) {

}

//get methods
float getSeaLevelPressure() {
    return 0.0;
}

//set methods
void setSeaLevelPressure(float pressure) {

}

//sensor reading functions
float getTemperature() {
    return Baro;
}
float getPressure() {
    return 
}
float getPressurePascal() {

}
float getBaroAltitude(float seaLevelPressure) {
    return 0.0;
}