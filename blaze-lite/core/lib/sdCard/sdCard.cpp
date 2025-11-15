#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
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
    File myFile = SD.open("example.txt", FILE_WRITE);
    if (myFile) {
        myFile.println("Hello, world!");
        myFile.close();
        Serial.println("Wrote to file.");
    } else {
        Serial.println("Error opening file.");
    }

    myFile = SD.open("example.txt");
    if (myFile) {
        Serial.println("example.txt:");
        while (myFile.available()) {
            Serial.write(myFile.read());
        }
        myFile.close();
    } else {
        Serial.println("Error opening file.");
    }
}



void loop()
{
    Serial.println("This is on the BlackPill board");
}

