#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <string>

class sdCard {
    public:
        //Constructors
        sdCard(const int csPin);

        //Destructor
        ~sdCard(); 

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
        ssize_t writeLog(const char* logEntry, const size_t length);

        ssize_t readLog(char* buffer, const size_t maxLength);

    private:
        char CS_PIN;
        String Datafile = "Data.txt";
        String Logfile = "Log.txt";
};