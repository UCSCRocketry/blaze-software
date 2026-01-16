#include "dataPacket.h"

#include <ctype.h>

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

uint32_t DataPacket::readUInt(const uint8_t* buf, size_t& offset, int nBytes)
{
    // Big-endian, read MSB first
    uint32_t val = 0;
    for (int i = 0; i < nBytes; i++) {
        val = (val << 8) | buf[offset++];
    }
    return val;
}

void DataPacket::writeSequenceId(uint32_t seq, uint8_t* buf, size_t& offset)
{
    uint32_t value = seq % 10000;
    buf[offset++] = (value / 1000) % 10;
    buf[offset++] = (value / 100) % 10;
    buf[offset++] = (value / 10) % 10;
    buf[offset++] = value % 10;
}

bool DataPacket::readSequenceId(const uint8_t* buf, size_t& offset, uint32_t& outSeq)
{
    uint8_t d0 = buf[offset++];
    uint8_t d1 = buf[offset++];
    uint8_t d2 = buf[offset++];
    uint8_t d3 = buf[offset++];

    if (d0 > 9 || d1 > 9 || d2 > 9 || d3 > 9) {
        return false;
    }

    outSeq = (d0 * 1000) + (d1 * 100) + (d2 * 10) + d3;
    return true;
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

bool DataPacket::checkCRC(const uint8_t* data, size_t len, uint16_t expectedCrc)
{
    uint16_t crc = computeCRC(data, len);
    return crc == expectedCrc;
}
void DataPacket::encodePacket(const uint8_t payload[PAYLOAD_SIZE],
                              char idA, char idB)
{
    size_t offset = 0;
    // 0. Clear buffer
    memset(buffer, 0, PACKET_SIZE);

    // 1. Start byte
    buffer[offset++] = static_cast<uint8_t>(startByte);

    // 2. Sequence ID (4 digits, each byte 0-9)
    writeSequenceId(sequenceID++, buffer, offset);

    // 3. Message ID (2 chars)
    if (idA >= 'A' && idA <= 'Z') idA = static_cast<char>(tolower(idA));
    if (idB >= 'A' && idB <= 'Z') idB = static_cast<char>(tolower(idB));
    if (idA < 'a' || idA > 'z') idA = 'a';
    if (idB < 'a' || idB > 'z') idB = 'a';
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

bool DataPacket::decodePacket(const uint8_t* rawPacket, size_t packetLen, DecodedPacket& decoded)
{   
    if (packetLen != PACKET_SIZE) {
        decoded.isValid = false;
        return false;
    }

    // Check end bytes (CR LF)
    if (rawPacket[packetLen - 2] != 0x0D || rawPacket[packetLen - 1] != 0x0A) {
        decoded.isValid = false;
        return false;
    }
    
    size_t offset = 0;
    
    // 1. Start byte
    uint8_t startByteVal = rawPacket[offset++];
    if (startByteVal != static_cast<uint8_t>(StartByte::NO_RESPONSE) &&
        startByteVal != static_cast<uint8_t>(StartByte::ACK_RESPONSE) &&
        startByteVal != static_cast<uint8_t>(StartByte::HUMAN_MESSAGE) &&
        startByteVal != static_cast<uint8_t>(StartByte::EXPECT_ACK)) {
        decoded.isValid = false;
        return false;
    }
    decoded.startByte = static_cast<StartByte>(startByteVal);

    // 2. Sequence ID (4 bytes, each 0-9)
    if (!readSequenceId(rawPacket, offset, decoded.sequenceID)) {
        decoded.isValid = false;
        return false;
    }

    // 3. Message ID (2 chars)
    decoded.idA = static_cast<char>(rawPacket[offset++]);
    decoded.idB = static_cast<char>(rawPacket[offset++]);
    if (decoded.idA < 'a' || decoded.idA > 'z' || decoded.idB < 'a' || decoded.idB > 'z') {
        decoded.isValid = false;
        return false;
    }

    // 4. Timestamp (4 bytes, big-endian)
    decoded.timestamp = readUInt(rawPacket, offset, 4);
    // 5. Payload (17 bytes)
    for (size_t i = 0; i < PAYLOAD_SIZE; i++) {
        decoded.payload[i] = rawPacket[offset++];
    }

    // 6. CRC-16 check
    decoded.crc = static_cast<uint16_t>(rawPacket[offset++] << 8);
    decoded.crc |= static_cast<uint16_t>(rawPacket[offset++]);
    if (!checkCRC(rawPacket, offset - 2, decoded.crc)) {
        decoded.isValid = false;
        return false;
    }
    decoded.isValid = true;
    return true;
}

