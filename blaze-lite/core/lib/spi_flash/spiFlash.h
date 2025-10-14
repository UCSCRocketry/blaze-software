/*
    spiFlash.h - a library that handles the Adafruit SPI flash breakout board for Blackpill Arduino
    To use:
        include this header file in your main program, DO NOT directly call the cpp file
*/

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>

#include <Adafruit_SPIFlash.h>

class spiFlash {
    public:
        //Constructor:
        spiFlash();

        //Get Methods:
        uint8_t getCS_PIN(uint8_t pin);

        //Set Methods:
        bool setCS_PIN(uint8_t pin);

    private:
        CS_PIN;

}



#endif