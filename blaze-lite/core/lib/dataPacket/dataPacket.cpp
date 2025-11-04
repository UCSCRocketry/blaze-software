#include "DATA_PACKET_H"

// Compute checksum (simple XOR or sum of bytes)
uint8_t calculateChecksum(const uint8_t* buffer, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum ^= buffer[i]; // XOR checksum (lightweight and fast)
    }
    return sum;
}

void buildPacket(uint8_t sensorID, const uint8_t* data, size_t dataLength, uint8_t* buffer) {
    // Start with a clean buffer
    memset(buffer, 0, PACKET_SIZE);

    // Header and Sensor ID
    buffer[0] = PACKET_HEADER;
    buffer[1] = sensorID;

    // Copy data bytes manually for endianness control
    for (size_t i = 0; i < dataLength && i + 2 < PACKET_SIZE - 1; i++) {
        buffer[i + 2] = data[i]; // store payload starting at index 2
    }

    // Add checksum at the end (excluding checksum byte)
    buffer[PACKET_SIZE - 1] = calculateChecksum(buffer, PACKET_SIZE - 1);
}

void sendPacket(const uint8_t* buffer, size_t length) {
    // Send over UART (transmitter group can replace with their own send function)
    Serial.write(buffer, length);
}