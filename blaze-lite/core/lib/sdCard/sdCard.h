#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <string>

class spiFlash;

class sdCard {
    public:
        //Constructors
        sdCard(const uint8_t csPin);
        //Destructor
        ~sdCard(); 
        //setup function
        void startUp();
        //Get Methods
        uint8_t getCS_PIN();
        //Set Methods
        void setCS_PIN(uint8_t pin);
        //data read/write methods
        ssize_t writeData(const size_t bytes, const char* data);
        ssize_t readData(const size_t bytes, char* buffer);
        //log read/write methods
        ssize_t writeLog(const char* logEntry, const size_t length);
        ssize_t readLog(char* buffer, const size_t maxLength);

        /**
         * Copy every regular file from SPI flash FAT root into destFolder on the SD card.
         * Creates destFolder if needed. Existing SD files with the same relative path are replaced.
         * Does not delete or alter files on SPI flash.
         */
        bool exportSpiFlashRootTo(spiFlash& flash, const char* destFolder);

    private:
        uint8_t CS_PIN;
        String Datafile = "Data.txt";
        String Logfile = "Log.txt";
};
