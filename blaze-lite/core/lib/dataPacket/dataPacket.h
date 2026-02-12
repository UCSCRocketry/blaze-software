#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <Arduino.h>
#include <stdint.h>

enum class StartByte : uint8_t {
    NO_RESPONSE        = '!',
    ACK_RESPONSE       = '"',
    HUMAN_MESSAGE      = '#',
    EXPECT_ACK         = '$'
};

// Decoded packet structure for easy access to packet fields
struct DecodedPacket {
    StartByte startByte;
    uint32_t sequenceID;
    char idA;
    char idB;
    uint32_t timestamp;
    uint8_t payload[17];
    uint16_t crc;
    bool isValid;  // true if packet decoded successfully
};

class DataPacket {
public:
    static constexpr size_t PAYLOAD_SIZE = 17;
    static constexpr size_t PACKET_SIZE =
        1 +           // start byte
        4 +           // sequence ID
        2 +           // message ID
        4 +           // timestamp (ms)
        PAYLOAD_SIZE +
        2 +           // CRC
        2;            // end bytes

    DataPacket(StartByte startType);

    /**
     * Encode an entire packet in one call.
     * payload MUST be exactly 17 bytes for the protocol.
     */
    void encodePacket(const uint8_t payload[PAYLOAD_SIZE], char idA, char idB);

    /**
     * Decode a raw packet buffer into a DecodedPacket structure.
     * Returns true if decode was successful, false otherwise.
     */
    bool decodePacket(const uint8_t* rawPacket, size_t packetLen, DecodedPacket& decoded);

    uint8_t* getBuffer();
    size_t getLength() const { return PACKET_SIZE; }

private:
    StartByte startByte;
    uint32_t sequenceID;
    uint8_t buffer[PACKET_SIZE];

    void writeUInt(uint32_t val, uint8_t* buf, size_t& offset, int nBytes);
    uint32_t readUInt(const uint8_t* buf, size_t& offset, int nBytes);
    void writeSequenceId(uint32_t seq, uint8_t* buf, size_t& offset);
    bool readSequenceId(const uint8_t* buf, size_t& offset, uint32_t& outSeq);
    uint16_t computeCRC(const uint8_t* data, size_t len);
    bool checkCRC(const uint8_t* data, size_t len, uint16_t expectedCrc);
};

#endif // DATA_PACKET_H