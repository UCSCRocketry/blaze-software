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
        void setCS_PIN(uint8_t pin);

        //functionality methods:
        ssize_t read(size_t offset, size_t bytes, uint8_t* buffer);

        uint8_t queue(size_t bytes, uint8_t* data, int priority = P_UNIMPORTANT);

        uint8_t buffer (size_t bytes, uint8_t* data);

        ssize_t write (size_t bytes, uint8_t* data);

        void flush (void);

        ssize_t kLog (size_t bytes, uint8_t* data);

    private:
        uint8_t CS_PIN; 

}

#endif