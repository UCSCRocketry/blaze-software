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

uint32_t DataPacket::readUInt(const uint8_t* buf, size_t& offset, int nBytes)
{
    // Big-endian, read MSB first
    uint32_t val = 0;
    for (int i = 0; i < nBytes; i++) {
        val = (val << 8) | buf[offset++];
    }
    return val;
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

uint16_t DataPacket::checkCRC(const uint8_t* data, size_t len)
{
    //data should include crc so length will be longer on this side
    uint16_t crc = 0xFFFF; // Initial value for CRC-16-CCITT

    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8; //Bitshift left 1 byte

        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) { // If MSB is 1
                crc = (crc << 1) ^ 0x1021; // XOR with polynomial
            } else {
                crc <<= 1; // Shift left
            }
        }
    }

    return crc == 0;
}
void DataPacket::encodePacket(const uint8_t payload[PAYLOAD_SIZE],
                              char idA, char idB)
{
    size_t offset = 0;
    // 0. Clear buffer
    memset(buffer, 0, PACKET_SIZE);

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

bool DataPacket::decodePacket(const uint8_t* rawPacket, size_t packetLen, DecodedPacket& decoded)
{   
    // Check end bytes (CR LF)
    if (rawPacket[packetLen - 2] != 0x0D || rawPacket[packetLen - 1] != 0x0A) {
        decoded.isValid = false;
        return false;
    }
    
    size_t offset = 0;
    
    // 1. Start byte
    uint8_t startByteVal = rawPacket[offset++];
    if (startByteVal < static_cast<uint8_t>(StartByte::NO_RESPONSE) || 
        startByteVal > static_cast<uint8_t>(StartByte::EXPECT_ACK)) {
        decoded.isValid = false;
        return false;
    }
    decoded.startByte = static_cast<StartByte>(startByteVal);
    // 2. Sequence ID (4 bytes, big-endian)
    decoded.sequenceID = readUInt(rawPacket, offset, 4);
    // 3. Message ID (2 chars)
    decoded.idA = static_cast<char>(rawPacket[offset++]);
    decoded.idB = static_cast<char>(rawPacket[offset++]);
    // 4. Timestamp (4 bytes, big-endian)
    decoded.timestamp = readUInt(rawPacket, offset, 4);
    // 5. Payload (17 bytes)
    for (size_t i = 0; i < PAYLOAD_SIZE; i++) {
        decoded.payload[i] = rawPacket[offset++];
    }
    // 6. CRC-16 check
    if (checkCRC(rawPacket, packetLen-2)!=0) {
        decoded.isValid = false;
        return false;
    }
    decoded.isValid = true;
    return true;
}

