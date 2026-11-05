/**
 * @file TemperatureSensor.h
 * @brief TMP121 temperature sensor implementation.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Converts TMP121 raw data into Celsius.
 * @param raw_data Raw 16-bit temperature data from TMP121.
 * @return Temperature in degrees Celsius.
 */
float convert_temperature(uint16_t raw_data);

int8_t read_temperature(uint16_t& raw_data);

int8_t get_temperature(float& raw_data);

