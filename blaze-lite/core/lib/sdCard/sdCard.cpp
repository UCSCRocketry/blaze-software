/*
    * sdCard.cpp - SD Card library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/

#include "sdCard.h"

File dataFile; // global data file object
File logFile;  // global log file object

sdCard::sdCard(const int csPin) {
    this->CS_PIN = csPin;
}

sdCard::~sdCard() {
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
    dataFile = SD.open("Data.txt", FILE_WRITE);
    if (!dataFile) {
        Serial.println("Error opening Data.txt");
    }
    logFile = SD.open("Log.txt", FILE_WRITE);
    if (!logFile) {
        Serial.println("Error opening Log.txt");
    }
}

char sdCard::getCS_PIN() {
    return this->CS_PIN;
}

void sdCard::setCS_PIN(char pin) {
    this->CS_PIN = pin;
}

ssize_t sdCard::writeData(const size_t bytes, const char* data) {
    if (dataFile) {
        size_t written = dataFile.write((const uint8_t*)data, bytes);
        dataFile.flush();
        return written;
    } else {
        return -1; // error
    }
}

ssize_t sdCard::readData(const size_t bytes, char* buffer) {
    if (dataFile) {
        dataFile.seek(0); // go to the beginning
        size_t readBytes = dataFile.readBytes(buffer, bytes);
        return readBytes;
    } else {
        return -1; // error
    }
}


