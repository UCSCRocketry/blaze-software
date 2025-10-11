#include <Arduino.h>
#include <iostream>
#include <cstring>
#include "RFD900x.h"

const long baudrate = 115200;
RFD900x rfd(Serial1);

void setup()
{
    Serial.begin(baudrate);
    while (!Serial) {
        delay(10);
    }
    Serial.println("This is on the BlackPill board");
    delay(10000);
    rfd.init(57600);
    Serial.println("RFD900x initialized");
    if (rfd.enterATMode(1200)) {
        Serial.println("Entered AT mode");
    } else {
        Serial.println("Failed to enter AT mode");
    }
    const char* cmd = "ATI\r";
    rfd.send((const uint8_t*)cmd, strlen(cmd));

    // Read response
    uint8_t buf[64] = {0};
    size_t n = rfd.recv(buf, sizeof(buf) - 1, 5000);
    if (n > 0) {
        Serial.print("Response: ");
        Serial.write(buf, n);
    } else {
        Serial.println("No response received.");
    }
    //const char* cmd2 = "ATO\r";
    //rfd.send((const uint8_t*)cmd2, strlen(cmd2));
    Serial1.println("ATO\r");
    uint8_t buf2[64] = {0};
    size_t n2 = rfd.recv(buf2, sizeof(buf2) - 1, 5000);
    if (n2 > 0) {
        Serial.print("Response: ");
        Serial.write(buf2, n2);
    } else {
        Serial.println("No response received.");
    }
}



void loop()
{
    delay(1000);
    Serial.println("Sending Hello from blackpill");
    const char* cmd = "Hello From Blackpill\n\r";
    rfd.send((const uint8_t*)cmd, strlen(cmd));
}