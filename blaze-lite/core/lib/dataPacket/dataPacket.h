#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <Arduino.h>

// Constants
#define PACKET_VERSION 1
#define PACKET_ID 0xAB  // optional ID for identifying your packet type

// Define DataPact structure
struct DataPacket {
    uint8_t id;
    uint8_t version;
    uint32_t timestamp; 
    float altitude;
    float acceleration;
    float temperature;
    uint8_t state;
    uint16_t checksum;
};

// return a created packet with filled data
DataPacket createPacket(float altitude, float acceleration, float temperature, uint8_t state);

// validate the packet's integrity with checksum
bool validatePacket(const DataPacket& packet);

// check if the packet has expected ID and version
bool checkID(const DataPacket& packet, uint8_t expectedID = PACKET_ID);

// decodes raw data into a DataPacket structure
DataPacket decodePacket(const uint8_t* buffer);

// Encodes a DataPacket into a byte buffer (for sending)
void encodePacket(DataPacket& packet, uint8_t* buffer);

// send the packet(to other groups)
void sendPacket(const DataPacket& packet);

// log to SD card
void logPacket(const DataPacket& packet);

#endif