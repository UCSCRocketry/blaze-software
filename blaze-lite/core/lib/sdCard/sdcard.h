#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SdFat.h>

#include <string>

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

        //data read/write methods
        ssize_t writeData(const size_t bytes, const char* data);

        ssize_t readData(const size_t bytes, char* buffer);

        //log read/write methods
        ssize_t writeLog(const char* logEntry);

        ssize_t readLog(char* buffer, const size_t maxLength);

    private:
        char CS_PIN;
        string Datafile = "Data.txt";
        string Logfile = "Log.txt";
};