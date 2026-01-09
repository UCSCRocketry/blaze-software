/*
    * sdCard.cpp - SD Card library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/

#include "sdCard.h"

File sdFile; // global file object

sdCard::SDCard(const int csPin, const size_t buffersize) {
    this->CS_PIN = csPin;
}

sdCard::~SDCard() {
    sdFile.close();
}

void sdCard::startUp() {
    //sd card init
    //serial begin should be called in the initialize state from state machine
    Serial.print("Initializing SD card...");
    if (!SD.begin(this->CS_PIN)) {
        Serial.println("Card failed, or not present");
        return;
    }
    Serial.println("SD Card initialized.");
    //open file
    sdFile = SD.open("Log.txt", FILE_WRITE);
    if (!sdFile) {
        Serial.println("Error opening Log.txt");
    }
    //test read&write
    sdFile.write("SD Card Test Log Entry\n");
    //check of the file is empty
    if (sdFile.size() == 0) {
        Serial.println("Log.txt cannot be read or was not able to be written to.");
    } else {
        Serial.println("Log.txt is ready for use.");
    }
}

char sdCard::getCS_PIN() {
    return this->CS_PIN;
}

void sdCard::setCS_PIN(char pin) {
    this->CS_PIN = pin;
}


