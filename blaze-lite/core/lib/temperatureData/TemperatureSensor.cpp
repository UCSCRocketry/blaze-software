/**
 * @file TemperatureSensor.cpp
 * @brief TMP121 temperature sensor implementation.
 */

#include "TemperatureSensor.h"
#include <Arduino.h>
#include <SPI.h>

const uint8_t CS_PIN = 10; // change value to what our CS PIN is?


float convert_temperature(uint16_t raw_data){
    /** tmp 121 data format:
     * 15-3 : temperature bits (the relevant info)
     * 2    : 0
     * 0-1  : Z
     */
    // shift data bits by 3, since the last 3 don't matter
     int16_t temp_data = raw_data >> 3; 

    // check if temperature is signed/negative
    if (temp_data & 0x1000){
        temp_data -= 0x2000;
    }

    return temp_data * 0.0625f; // return temp, convert to ºC 
}

int8_t read_temperature(uint16_t& raw_data){
    // setup
    SPI.beginTransaction(
        SPISettings(1000000, MSBFIRST, SPI_MODE0)
    );
    
    digitalWrite(CS_PIN, LOW); // select TMP121

    // get data from TMP121
    uint8_t high_byte = SPI.transfer(0x00); // give random input (input not meaningful)
    uint8_t low_byte = SPI.transfer(0x00);  // same thing

    // done reading, deselect TMP121
    digitalWrite(CS_PIN, HIGH);

    SPI.endTransaction(); // read is finished!

    // combine the values of the read together
    raw_data = (uint16_t(high_byte) << 8) | low_byte; 

    return 0; // success
}

int8_t get_temperature(float& temperature){
    uint16_t raw_data = 0;

    // read temperature
    int8_t status = read_temperature(raw_data);
    if (status != 0){ // there was an error!!!
        return status;
    }

    // convert temperature
    temperature = convert_temperature(raw_data);

    return 0; // success!
}

