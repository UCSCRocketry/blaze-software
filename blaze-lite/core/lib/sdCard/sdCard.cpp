#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <sdFat.h>
#define SD_CARD_H
  #include <queue> 
  #include <tuple>
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include <cmath>
  #include <vector>
  #include <fstream>
  #include <iostream>
  typedef std::ptrdiff_t ssize_t;
  #else
  #include <sys/types.h>

// Persistent file handle + in-memory write buffer for batched writes.
static File persistentFile;
static std::vector<uint8_t> writeBuffer;
static const size_t WRITE_BUFFER_CAPACITY = 512; // tuneable buffer size
// Periodic flush timer values
static unsigned long lastFlushMillis = 0;
static const unsigned long FLUSH_INTERVAL_MS = 5000; // flush every 5s by default

const long baudrate = 115200;
const int chipSelect = 3;

void setup()
{
    Serial.begin(baudrate);
    while (!Serial) {
        delay(10);
    }
    Serial.println("Initializing SD Card...");

    if (!SD.begin()) {
        Serial.println("Card failed, or not present");
        return;
    }
    Serial.println("Card initialized.");
    // Open a persistent file handle for buffered writes. Keep it open
    // between writes to avoid repeatedly opening/closing the file.
    persistentFile = SD.open("example.txt", FILE_WRITE);
    if (!persistentFile) {
        Serial.println("Warning: could not open persistent file for buffered writes.");
    } else {
        // Seek to end so we append
        persistentFile.seek(persistentFile.size());
        Serial.println("Persistent file opened for buffered writes.");
    }

    // Example: append a line (goes into buffer, not necessarily immediately to card)
    buffer(14, "Hello, world!\n");
    // Optionally flush here to force commit
    flush();
}

ssize_t read(const size_t offset, const size_t bytes, char* buffer) {
    File myFile = SD.open("example.txt");
    if (!myFile) {
        Serial.println("Error opening file for reading.");
        return -1;
    }
    myFile.seek(offset);
    ssize_t bytesRead = myFile.readBytes(buffer, bytes);
    myFile.close();
    return bytesRead;
}

ssize_t write(const size_t bytes, const char* data) {
    // Buffered write: append to in-memory buffer and flush to the SD card
    // when the buffer exceeds WRITE_BUFFER_CAPACITY.
    if (bytes == 0) return 0;
    // Append data to buffer
    try {
        writeBuffer.insert(writeBuffer.end(), (const uint8_t*)data, (const uint8_t*)data + bytes);
    } catch (...) {
        Serial.println("Buffer insert failed.");
        return -1;
    }

    // If buffer big enough, flush to card
    if (writeBuffer.size() >= WRITE_BUFFER_CAPACITY) {
        if (flush() != 0) {
            return -1;
        }
    }

    return (ssize_t)bytes;
}

char buffer (const size_t bytes, const char* data) {
    return write(bytes, data) >= 0 ? 0 : -1;
}

char flush (void) {
    // Write any buffered bytes to the persistentFile and force a commit by
    // closing and reopening the file. Returns 0 on success, -1 on failure.
    if (writeBuffer.empty()) return 0; // nothing to do

    // Ensure persistent file is open
    if (!persistentFile) {
        persistentFile = SD.open("example.txt", FILE_WRITE);
        if (!persistentFile) {
            Serial.println("Error opening persistent file to flush.");
            return -1;
        }
        persistentFile.seek(persistentFile.size());
    }

    // Write buffered data
    ssize_t written = persistentFile.write(writeBuffer.data(), writeBuffer.size());
    if (written < 0 || (size_t)written != writeBuffer.size()) {
        Serial.println("Error writing buffered data to SD card.");
        return -1;
    }

    // Close then reopen to ensure card controller commits.
    persistentFile.close();
    // Reopen for further buffered writes (append mode)
    persistentFile = SD.open("example.txt", FILE_WRITE);
    if (!persistentFile) {
        Serial.println("Error reopening persistent file after flush.");
        return -1;
    }
    persistentFile.seek(persistentFile.size());

    writeBuffer.clear();
    return 0;
}



void loop() // emptying buffer into file; runs repeatedly on Arduino
{
    unsigned long now = millis();

    // Periodically flush buffered writes to the SD card
    if ((now - lastFlushMillis) >= FLUSH_INTERVAL_MS) {
        if (!writeBuffer.empty()) {
            if (flush() == 0) {
                Serial.println("Buffered data flushed to SD card.");
            } else {
                Serial.println("Buffered flush failed.");
            }
        }
        lastFlushMillis = now;
    }

    // Allow manual flush via serial command: send "FLUSH" followed by newline
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.equalsIgnoreCase("FLUSH")) {
            Serial.println("Manual flush requested");
            if (flush() == 0) Serial.println("Manual flush succeeded");
            else Serial.println("Manual flush failed");
        }
    }

    // Small idle delay to avoid busy-looping; tune based on application
    delay(10);
}

