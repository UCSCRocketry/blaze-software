/*
    * spiFlash.cpp - SPI Flash library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/
#include "spiFlash.h"

//Constructor:
spiFlash::spiFlash() {
    //Default Constructor
    CS_PIN = 0; //sets default Chip Select pin to 0

    Adafruit_FlashTransport_SPI flashTransport(CS_PIN, SPI);
    Adafruit_SPIFlash flash(&flashTransport);
}

//Get Methods:
uint8_t spiFlash::getCS_PIN(uint8_t pin) {
    return CS_PIN;
}

//Set Methods:
void spiFlash::setCS_PIN(uint8_t pin) {
    CS_PIN = pin;
}

