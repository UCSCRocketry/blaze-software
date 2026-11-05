#include "gps.h"


/** Sets up the I2C connection to our GPS */
void GPS::setUpI2C() {
    Wire.begin(); // Initialize I2C communication
    Wire.setClock(400000); // Set I2C clock speed to 400kHz (fast mode)
}


/** Sends a UBX message. 
 * @param message Pointer to the UBX message to send.
 * @param length Length of the UBX message in bytes.
 * @returns true if successful, false otherwise.
 */
bool GPS::sendUBXMessage(const uint8_t* message, size_t length) {
    Wire.beginTransmission(GPS_I2C_ADDRESS);

    size_t numBytes = Wire.write(message, length);

    uint8_t status = Wire.endTransmission();
    return status == 0 && numBytes == length; // Return true if transmission was successful and complete.
}


/** Reads a UBX message from the GPS module, and updates the provided address with the data.
 * @param addr Pointer to the buffer to store the received message.
 * @param length Length of the buffer in bytes.
 */
void GPS::readBytes(uint8_t* addr, size_t length) {
    Wire.requestFrom(GPS_I2C_ADDRESS, length);
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < length) {
        addr[bytesRead++] = Wire.read();
    }
}


/** Parses a single UBX byte and updates the parser state accordingly. */
void GPS::parseUBXByte(uint8_t byte) {
    switch (parserState) {
        case SYNC1:
            if (byte == 0xB5) {
                parserState = SYNC2;
            }
            break;
        case SYNC2:
            if (byte == 0x62) {
                parserState = CLASS;
            } else {
                parserState = SYNC1; // Reset to SYNC1 if not matching
            }
            break;
        case CLASS: // past this point we'll read the whole message.
            // init
            currentMessage.msgSize = 0;
            currentMessage.msgIndex = 0;
            currentMessage.msgClass = byte;
            parserState = ID;
            break;
        case ID:
            currentMessage.msgId = byte;
            parserState = LENGTH1;
            break;
        case LENGTH1:
            currentMessage.msgSize += byte; // little endian
            parserState = LENGTH2;
            break;
        case LENGTH2:
            currentMessage.msgSize += (byte << 8); // multiply by 256 to get the high byte
            parserState = DATA;
            break;
        case DATA:
            if(currentMessage.msgIndex < currentMessage.msgSize) {
                payload[currentMessage.msgIndex] = byte;
                currentMessage.msgIndex++;

                if(currentMessage.msgIndex == currentMessage.msgSize) {
                    parserState = CHECKSUM_A; // move to checksum after reading all data
                }
            }
            break;
        case CHECKSUM_A:
            // Handle checksum A byte
            parserState = CHECKSUM_B;
            break;
        case CHECKSUM_B:
            // Handle checksum B byte and reset to SYNC1 for next message
            parserState = SYNC1;
            break;
    }
}