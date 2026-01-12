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
    delay(500);
    
    rfd.init(57600);
    Serial.println("RFD900x initialized");

    //Always exit AT mode on startup in case it's on AT mode, otherwise, there's no way to tell it to get out of AT mode.
    Serial1.println("ATO\r");
}

void loop()
{
    delay(2000);

    char resp[32] = {0};
    rfd.recv((uint8_t*)resp, sizeof(resp)-1);
    rfd.handleATReceiving(resp);

    Serial.println("Sending Hello from blackpill");
    const char* cmd = "Hello From Blackpill\n\r";
    rfd.send((const uint8_t*)cmd, strlen(cmd));
}