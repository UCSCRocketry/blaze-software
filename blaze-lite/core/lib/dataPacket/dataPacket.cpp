#include "dataPacket.h"

DataPacket::DataPacket(StartByte startType)
    : startByte(startType), sequenceID(0)
{
    memset(buffer, 0, PACKET_SIZE);
}

void DataPacket::writeUInt(uint32_t val, uint8_t* buf, size_t& offset, int nBytes)
{
    // Big-endian, write MSB first
    for (int i = nBytes - 1; i >= 0; --i) {
        buf[offset++] = (val >> (8 * i)) & 0xFF;
    }
}

uint16_t DataPacket::computeCRC(const uint8_t* data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

void DataPacket::encodePacket(const uint8_t payload[PAYLOAD_SIZE],
                              char idA, char idB)
{
    size_t offset = 0;

    // 1. Start byte
    buffer[offset++] = static_cast<uint8_t>(startByte);

    // 2. Sequence ID (big-endian uint32)
    writeUInt(sequenceID++, buffer, offset, 4);

    // 3. Message ID (2 chars)
    buffer[offset++] = static_cast<uint8_t>(idA);
    buffer[offset++] = static_cast<uint8_t>(idB);

    // 4. Timestamp
    writeUInt(millis(), buffer, offset, 4);

    // 5. Payload (17 bytes, written manually)
    for (size_t i = 0; i < PAYLOAD_SIZE; i++) {
        buffer[offset++] = payload[i];
    }

    // 6. CRC-16 over all prior bytes
    uint16_t crc = computeCRC(buffer, offset);
    buffer[offset++] = (crc >> 8) & 0xFF;
    buffer[offset++] = crc & 0xFF;

    // 7. End bytes <CR><LF>
    buffer[offset++] = 0x0D;
    buffer[offset++] = 0x0A;
}

uint8_t* DataPacket::getBuffer() {
    return buffer;
}
