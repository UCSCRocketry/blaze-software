#include "tester.h"

//Constructor
tester::tester() {}

//Test functions
/*
    Function to check the validity of a packet
    Protocol:
    Byte 0: Start byte (0x21 ~ 0x24)
    Byte 1~4: Packet Sequence ID (0x00 ~ 0x09)
    Byte 5~6: Message Type ID (2 alphabetic lower case letters)
    Byte 7~10: Timestamp 
    Byte 11~27: Payload (Any value)
    Byte 28~29: Error Check (TO-DO: idk how to do this)
    Byte 30~31: End byte (Exact format: 0x0D 0x0A)

    Functions:
        Check if the packet is 32 bytes long
        Check the range of each field of the packet

    Input:  
        Pointer to a packet
        Length to read
    Output:
        True if the packet is valid, false otherwise
*/
bool tester::protocolTest(const uint8_t* inComingPacket, size_t packetLength) {
    bool isValid = true;
    //print incoming packet
    Serial.println("Incoming Packet:");
    for (size_t i = 0; i < packetLength; i++) {
        Serial.print(inComingPacket[i], HEX);
        Serial.print(" ");
    }

    Serial.println();

    //check length
    if (packetLength != 32) {
        Serial.println("Invalid packet length");
        isValid = false;
    }

    //only check others if length is valid
    if (isValid){
        //check start byte
        if (inComingPacket[0] < 0x21 || inComingPacket[0] > 0x24) {
            Serial.println("Invalid start byte, must one of ! ” # $ ");
            isValid = false;
        }
        //check packet sequence ID
        for (int i = 0; i < 4; i++){
            if (inComingPacket[1 + i] < 0x00 || inComingPacket[1 + i] > 0x09) {
                Serial.println("Invalid packet sequence ID, must be 4 digits between 0 and 9");
                isValid = false;
            }
        }
        //check message type ID
        if (!((inComingPacket[5] >= 'a' && inComingPacket[5] <= 'z') && (inComingPacket[6] >= 'a' && inComingPacket[6] <= 'z'))) {
            Serial.println("Invalid message type ID, must be 2 lowercase letters");
            isValid = false;
            //check if its the IMU data type
            if (!(inComingPacket[5] == 'i' && inComingPacket[6] == 'm')) {
                Serial.println("Invalid message type ID, must be 'im' for IMU data");
                isValid = false;
            }
        }
        //check timestamp
        for (int i = 0; i < 4; i++){
            if (inComingPacket[7 + i] < 0x00 || inComingPacket[7 + i] > 0xFF) {
                Serial.println("Invalid timestamp, must be 4 bytes of integers between 0 and 9");
                isValid = false;
            }
        }
        //TO-DO: check CRC-16

        //check end byte
        if (!(inComingPacket[30] == 0x0D && inComingPacket[31] == 0x0A)) {
            Serial.println("Invalid end byte, must be <CR><LF>");
            isValid = false;
        }
    }

    //passed all checks
    if (isValid) {
        Serial.println("Packet is valid");
    }
    return isValid;
}