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


/** Waits for an acknowledgment from the GPS module after sending a UBX message. */
UbxAckType GPS::waitAcknowledgment(uint32_t timeoutMs){
    uint32_t start_t = millis();
    while (millis() - start_t < timeoutMs) {
        Wire.requestFrom(GPS_I2C_ADDRESS, ACK_MSG_LENGTH);
        size_t bytesRead = 0;
        bool ackFlag = false;
        while (Wire.available() && bytesRead < ACK_MSG_LENGTH) {
            uint8_t byte = Wire.read();
            if (bytesRead == 2 && byte != 0x05){
                break; // discard the packet
            }
            if (bytesRead == 3 && (byte != 0x01 && byte != 0x00)){
                break;
            }

            ackFlag = (bytesRead == 3 && byte == 0x01)
                ? true
                : ackFlag;
                
            bytesRead++;
        }
    }

    return UBX_TIMEOUT; // if timeout occurs, return timeout.
}