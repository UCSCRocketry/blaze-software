#include <Arduino.h>
#include <iostream>
#include <cstring>

const long baudrate = 115200;

void setup()
{
    Serial.begin(baudrate);
    while (!Serial) {
        delay(10);
    }
    Serial.println("This is on the BlackPill board");
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