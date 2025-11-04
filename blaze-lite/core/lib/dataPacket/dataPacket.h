#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <Arduino.h>

// Constants
#define PACKET_HEADER 0xAA // Start byte for UART synchronization
#define PACKET_SIZE 17  // Fixed payload size per sensor

// Function declarations

// Builds and encodes a packet for transmission
// sensorID: unique ID for sensor (e.g., 0x01 for barometer)
// data: pointer to raw sensor bytes (float, int, etc.)
// dataLength: number of bytes in sensor data
// buffer: destination byte array to hold full packet
void buildPacket(uint8_t sensorID, const uint8_t* data, size_t dataLength, uint8_t* buffer);

// Sends the packet via Serial (or to transmitter)
void sendPacket(const uint8_t* buffer, size_t length);

// Calculates checksum for error detection
uint8_t calculateChecksum(const uint8_t* buffer, size_t length);

#endif