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
    //might not need this
}

void sdCard::startUp() {
    //sd card init
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
}

char sdCard::getCS_PIN() {
    return this->CS_PIN;
}

void sdCard::setCS_PIN(char pin) {
    this->CS_PIN = pin;
}


