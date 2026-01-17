/*
    * sdCard.cpp - SD Card library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/

#include "sdcard.h"

File dataFile; // global data file object
File logFile;  // global log file object

sdCard::sdCard(const uint8_t csPin) {
    this->CS_PIN = csPin;
}

sdCard::~sdCard() {
    dataFile.close();
    logFile.close();
}

void sdCard::startUp() {
    //sd card init
    //serial begin should be called in the initialize state from state machine
    Serial.println("Initializing SD card...");
    SPI.begin(); //this would be started in state machine, this is just for testing
    pinMode(this->CS_PIN, OUTPUT);
    digitalWrite(this->CS_PIN, HIGH);
    delay(100); // Allow SD card to power up
    
    if (!SD.begin(this->CS_PIN)) {
        Serial.println("SD card failed to connect. Reason: failed to connect to SD breakout board, check CS pin");
        return;
    }
    Serial.println("SD Card initialized.");

    //open file
    //TODO: change generic file names later
    dataFile = SD.open("Data.txt", FILE_WRITE);
    if (!dataFile) {
        Serial.println("Error opening Data.txt");
    }
    logFile = SD.open("Log.txt", FILE_WRITE);
    if (!logFile) {
        Serial.println("Error opening Log.txt");
    }
}

uint8_t sdCard::getCS_PIN() {
    return this->CS_PIN;
}

void sdCard::setCS_PIN(uint8_t pin) {
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

ssize_t sdCard::writeLog(const char* logEntry, const size_t length) {
    if (logFile) {
        size_t written = logFile.write((const uint8_t*)logEntry, length);
        logFile.flush();
        return written;
    } else {
        return -1; // error
    }
}

ssize_t sdCard::readLog(char* buffer, const size_t maxLength) {
    if (logFile) {
        logFile.seek(0); // go to the beginning
        size_t readBytes = logFile.readBytes(buffer, maxLength);
        return readBytes;
    } else {
        return -1; // error
    }
}
