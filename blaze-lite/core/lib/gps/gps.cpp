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