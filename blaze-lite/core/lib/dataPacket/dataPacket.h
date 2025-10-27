#include <Arduino.h>

// Define DataPact structure
struct DataPacket {
    uint8_t version;
    uint32_t timestamp; 
    float altitude;
    float acceleration;
    float temperature;
    uint8_t state;
    uint16_t checksum;
};

// return a created packet
DataPacket createPacket();

// validate the packet's integrity with checksum
bool validatePacket(const DataPacket& packet);

// check if the packet ID matches
bool checkID(const DataPacket& packet, int id);

// decode the packet data
void decodePacket(const DataPacket& packet);

// send the packet(to other groups)
void sendPacket(const DataPacket& packet);

// log to SD card
void logPacket(const DataPacket& packet);