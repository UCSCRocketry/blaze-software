#include "Baro.h"
#include <MS5611.h>

Baro::Baro(uint8_t addressPin){

}

int Baro::read() {
    
}

bool startUp() {
    
}


uint32_t getDeviceID() {
    
}
uint16_t getProm(uint8_t index) {
    
}
uint16_t getCRC() {
    
}

//configuration functions
void setOverSamplingRate(uint8_t osr) {
    
}

void setPressureOffset(float offset) {
    

}

//get methods
float getSeaLevelPressure() {
    
}

//set methods
void setSeaLevelPressure(float pressure) {

}

//sensor reading functions
float getTemperature() {
    
}
float getPressure() {
     
}
float getPressurePascal() {

}
float getBaroAltitude(float seaLevelPressure) {
    
}