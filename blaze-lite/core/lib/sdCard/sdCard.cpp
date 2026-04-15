/*
    * sdCard.cpp - SD Card library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/

#include "sdCard.h"

#include "spiFlash.h"

#include <stdio.h>
#include <string.h>

File dataFile; // global data file object
File logFile;  // global log file object

namespace {
void makeDataFileName(char* buffer, size_t bufferSize) {
    uint16_t fileIndex = 0;
    do {
        snprintf(buffer, bufferSize, "DATA%03u.txt", fileIndex++);
    } while (SD.exists(buffer) && fileIndex < 1000);
}
void makeLogFileName(char* buffer, size_t bufferSize) {
    uint16_t fileIndex = 0;
    do {
        snprintf(buffer, bufferSize, "LOG%03u.txt", fileIndex++);
    } while (SD.exists(buffer) && fileIndex < 1000);
}

struct SdExportState {
    const char* folder;
    File out;
    bool open;
};

static bool sdExportBegin(void* user, const char* filename) {
    auto* st = static_cast<SdExportState*>(user);
    char path[96];
    if (st->folder != nullptr && st->folder[0] != '\0') {
        if (snprintf(path, sizeof(path), "%s/%s", st->folder, filename) >= static_cast<int>(sizeof(path))) {
            return false;
        }
    } else {
        if (snprintf(path, sizeof(path), "%s", filename) >= static_cast<int>(sizeof(path))) {
            return false;
        }
    }
    if (st->open) {
        st->out.close();
        st->open = false;
    }
    if (SD.exists(path)) {
        SD.remove(path);
    }
    st->out = SD.open(path, FILE_WRITE);
    st->open = static_cast<bool>(st->out);
    return st->open;
}

static bool sdExportWrite(void* user, const uint8_t* data, size_t len) {
    auto* st = static_cast<SdExportState*>(user);
    if (!st->open) {
        return false;
    }
    return st->out.write(data, len) == len;
}

static bool sdExportEnd(void* user) {
    auto* st = static_cast<SdExportState*>(user);
    if (st->open) {
        st->out.close();
        st->open = false;
    }
    return true;
}
}

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
    //SPI.begin(); //this would be started in state machine, this is just for testing
    pinMode(this->CS_PIN, OUTPUT);
    digitalWrite(this->CS_PIN, HIGH);
    delay(2000); // Allow SD card to power up
    
    if (!SD.begin(this->CS_PIN)) {
        Serial.println("SD card failed to connect. Reason: failed to connect to SD breakout board, check CS pin");
        return;
    }
    Serial.println("SD Card initialized.");

    //create data file that has a unique name with a integer, if file already excist, integer +1
    char dataFileName[16];
    char logFileName[16];

    makeDataFileName(dataFileName, sizeof(dataFileName));
    delay(500);
    makeLogFileName(logFileName, sizeof(logFileName));
    delay(500);

    dataFile = SD.open(dataFileName, FILE_WRITE);
    logFile = SD.open(logFileName, FILE_WRITE);
    if (!dataFile) {
        Serial.println("Failed to create data file on SD card");
    } else {
        Serial.print("Data file created: ");
        Serial.println(dataFileName);
    }
    if (!logFile) {
        Serial.println("Failed to create log file on SD card");
    } else {
        Serial.print("Log file created: ");
        Serial.println(logFileName);
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

bool sdCard::exportSpiFlashRootTo(spiFlash& flash, const char* destFolder) {
    // Requires SD.begin (e.g. sdCard::startUp) already succeeded; do not call SD.begin here
    // while dataFile/logFile may be open.
    if (destFolder != nullptr && destFolder[0] != '\0' && !SD.exists(destFolder)) {
        if (!SD.mkdir(destFolder)) {
            return false;
        }
    }

    SdExportState st;
    st.folder = destFolder;
    st.open = false;

    SpiFlashExportCallbacks cb;
    cb.user = &st;
    cb.onBeginFile = sdExportBegin;
    cb.onWrite = sdExportWrite;
    cb.onEndFile = sdExportEnd;

    bool ok = flash.exportRootFiles(&cb);
    if (st.open) {
        st.out.close();
    }
    return ok;
}
