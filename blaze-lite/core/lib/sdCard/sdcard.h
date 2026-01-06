#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SdFat.h>

class SDCard {
    public:
        //Constructors
        SDCard(const int csPin, const size_t buffersize);

        //Destructor
        ~SDCard(); 

        //setup function
        void startUp();

        //Get Methods
        char getCS_PIN();

        //Set Methods
        void setCS_PIN(char pin);

        //functionality methods
        ssize_t write(const size_t bytes, const char* data);
        
        ssize_t read(const size_t offset, const size_t bytes, char* buffer);

    private:
        char CS_PIN;
};