#include <Arduino.h>
#include <iostream>
#include <cstring>
#include <SPI.h>
#include "Radio.h"

#define RF69_FREQ 433.0
#define RFM69_CS    PB6
#define RFM69_INT   PB4
#define RFM69_RST   PB7

Radio rf69(RFM69_CS, RFM69_INT, RFM69_RST);

void setup()
{

    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("This is on the BlackPill board");
    delay(500);

    if (!rf69.init(RF69_FREQ)) {
        while (1);
    }
    Serial.println("RFM69 radio init OK!");

    //rf69.setCallSign("KO6LRX");
}

void loop()
{
    // Define the message to send
    const char *message = "Hello, World Meow :3";  // Send the message
    Serial.print("Sending: ");
    Serial.println(message);  rf69.send((uint8_t *)message, strlen(message));  // Transmit the message  // Wait until the message has been fully transmitted
    Serial.println("Message sent!");  // Wait a bit before sending the next message

    char resp[32] = {0};
    size_t r = rf69.recv((uint8_t*)resp, sizeof(resp)-1, 1000);
    String s = String(resp);
    Serial.println("Reply: " + s);

    delay(5000);

    // 猫 :3 - Rachelle
    // ニャン - Oscar
    // no meow - Alejandro
}