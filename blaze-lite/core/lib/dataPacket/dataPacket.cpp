#include "DATA_PACKET_H"

// Helper function (private scope) to calculate checksum
static uint16_t calculateChecksum(const uint8_t* data, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return ~sum;  // invert bits for simple error detection
}

// Core function implementations
DataPacket createPacket(float altitude, float acceleration, float temperature, uint8_t state) {
    DataPacket packet;

    packet.id = PACKET_ID;
    packet.version = PACKET_VERSION;
    packet.timestamp = millis();
    packet.altitude = altitude;
    packet.acceleration = acceleration;
    packet.temperature = temperature;
    packet.state = state;

    // Compute checksum (excluding checksum field itself)
    packet.checksum = calculateChecksum((uint8_t*)&packet, sizeof(DataPacket) - sizeof(packet.checksum));

    return packet;
}

bool validatePacket(const DataPacket& packet) {
    uint16_t calculated = calculateChecksum((uint8_t*)&packet, sizeof(DataPacket) - sizeof(packet.checksum));
    return calculated == packet.checksum;
}

bool checkID(const DataPacket& packet, uint8_t expectedID) {
    return (packet.id == expectedID && packet.version == PACKET_VERSION);
}

// Encoding / Decoding functions
void encodePacket(const DataPacket& packet, uint8_t* buffer) {
    memcpy(buffer, &packet, sizeof(DataPacket));
}

DataPacket decodePacket(const uint8_t* buffer) {
    DataPacket packet;
    memcpy(&packet, buffer, sizeof(DataPacket));
    return packet;
}

// Communication and Logging functions
// This function sends the packet via Serial (for transmitter group)
void sendPacket(const DataPacket& packet) {
    uint8_t buffer[sizeof(DataPacket)];
    encodePacket(packet, buffer);

    // Transmit bytes — adjust as needed for SPI or LoRa
    Serial.write(buffer, sizeof(DataPacket));
}

// This function logs packet data to SD (for SD card group)
void logPacket(const DataPacket& packet) {
    // Example logging — replace with actual SD library write
    Serial.print("LOG | Time: ");
    Serial.print(packet.timestamp);
    Serial.print(" Alt: ");
    Serial.print(packet.altitude);
    Serial.print(" Accel: ");
    Serial.print(packet.acceleration);
    Serial.print(" Temp: ");
    Serial.print(packet.temperature);
    Serial.print(" State: ");
    Serial.println(packet.state);
}