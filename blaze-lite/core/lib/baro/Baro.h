/**
 * @file Baro.h
 * @brief Wrapper class for MS5611 barometer using SPI communication
 * 
 * This class provides a simplified interface for interacting with the MS5611
 * barometric pressure sensor using SPI communication.
 */

#pragma once
#include <Arduino.h>
#include <MS5611_SPI.h>

/**
 * @class Baro
 * @brief Wrapper class for MS5611 barometer SPI communication
 */
class Baro {
public:
  /**
   * @brief Constructor for the Baro class
   * @param CS The Chip Select pin used for SPI communication
   */
  explicit Baro(uint8_t CS);
  
  /**
   * @brief Initializes the barometer
   * @return true if initialization was successful, false otherwise
   */
  bool init();

  /**
   * @brief Check if barometer is initialized and ready
   * @return true if ready, false otherwise
   */
  bool isReady() const;

  /**
   * @brief Triggers a read operation from the sensor
   * @return Status of the read operation
   */
  int read();

  /**
   * @brief Retrieves the last read pressure value
   * @return Pressure in millibars (mbar) / hPa
   */
  float getPressure();

  /**
   * @brief Retrieves the last read temperature value
   * @return Temperature in degrees Celsius
   */
  float getTemperature();

  /**
   * @brief Retrieves the altitude
   * @return Altitude in meters
   */
  float getAltitude();

  /**
   * @brief Retrieves the device ID of the barometer
   * @return Device ID byte
   */
  uint8_t getDeviceID();

private:
  MS5611_SPI baro;   ///< Underlying MS5611 driver object
  uint8_t cs_pin;    ///< Chip Select pin number
  bool _initialized; ///< Initialization status flag
};
