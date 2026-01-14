#include "Baro.h"

static const float DEFAULT_SEA_LEVEL_PRESSURE_MBAR = 1013.25f;

Baro::Baro(uint8_t csPin)
	: baroObject(csPin),
	  CS_PIN(csPin),
	  seaLevelPressure(DEFAULT_SEA_LEVEL_PRESSURE_MBAR) 
      { }

Baro::~Baro() = default;

bool Baro::startUp() {
    //SPI begin will be called in the state machine
	return baroObject.begin();
}

int Baro::read()
{
	return baroObject.read();
}

uint32_t Baro::getDeviceID()
{
	return baroObject.getDeviceID();
}

void Baro::setOverSamplingRate(uint8_t osr) {
	if (osr < (uint8_t)OSR_ULTRA_LOW) osr = (uint8_t)OSR_ULTRA_LOW;
	if (osr > (uint8_t)OSR_ULTRA_HIGH) osr = (uint8_t)OSR_ULTRA_HIGH;
	baroObject.setOversampling(static_cast<osr_t>(osr));
}

float Baro::getSeaLevelPressure() {
	return seaLevelPressure;
}

void Baro::setSeaLevelPressure(float pressure) {
	seaLevelPressure = pressure;
}

float Baro::getTemperature() {
	baroObject.read();
	return baroObject.getTemperature();
}

float Baro::getPressure() {
	baroObject.read();
	return baroObject.getPressure();
}

float Baro::getBaroAltitude(float seaLevelPressure) {
	baroObject.read();
	return baroObject.getAltitude(seaLevelPressure);
}
