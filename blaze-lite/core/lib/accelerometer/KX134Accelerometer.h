/**
 * @file KX134Accelerometer.h
 * @brief Wrapper class for SparkFun KX134 accelerometer using SPI communication
 * 
 * This class provides a simplified interface for interacting with the KX134
 * accelerometer using SPI communication. It wraps the SparkFun_KX134_SPI class
 * from the SparkFun_KX13X_Arduino_Library.
 */

#pragma once

#include <SPI.h>
#include <SparkFun_KX13X.h>

/**
 * @class KX134Accelerometer
 * @brief Wrapper class for KX134 accelerometer SPI communication
 */
class KX134Accelerometer {
public:
    /**
     * @brief Constructor
     */
    KX134Accelerometer();

    /**
     * @brief Destructor
     */
    ~KX134Accelerometer();

    /**
     * @brief Initialize the accelerometer with default SPI settings
     * @param csPin Chip select pin number
     * @return true if initialization successful, false otherwise
     */
    bool begin(uint8_t csPin);

    /**
     * @brief Initialize the accelerometer with custom SPI settings
     * @param spiPort SPI port to use
     * @param spiSettings SPI settings (speed, bit order, mode)
     * @param csPin Chip select pin number
     * @return true if initialization successful, false otherwise
     */
    bool begin(SPIClass &spiPort, SPISettings spiSettings, uint8_t csPin);

    /**
     * @brief Check if accelerometer is initialized and ready
     * @return true if ready, false otherwise
     */
    bool isReady() const;

    /**
     * @brief Perform software reset on the accelerometer
     * @return true if reset successful, false otherwise
     */
    bool reset();

    /**
     * @brief Enable or disable the accelerometer
     * @param enable true to enable, false to disable
     * @return true if operation successful, false otherwise
     */
    bool enable(bool enable = true);

    /**
     * @brief Set the measurement range
     * @param range Range setting (SFE_KX134_RANGE8G, SFE_KX134_RANGE16G, 
     *              SFE_KX134_RANGE32G, SFE_KX134_RANGE64G)
     * @return true if operation successful, false otherwise
     */
    bool setRange(uint8_t range);

    /**
     * @brief Enable the data engine
     * @param enable true to enable, false to disable
     * @return true if operation successful, false otherwise
     */
    bool enableDataEngine(bool enable = true);

    /**
     * @brief Set the output data rate (ODR)
     * @param odr Output data rate setting (see SparkFun_KX13X_regs.h for values)
     * @return true if operation successful, false otherwise
     */
    bool setOutputDataRate(uint8_t odr);

    /**
     * @brief Get the current output data rate
     * @return Output data rate in Hz
     */
    float getOutputDataRate();

    /**
     * @brief Check if new data is ready
     * @return true if data ready, false otherwise
     */
    bool dataReady();

    /**
     * @brief Get accelerometer data in g units
     * @param data Pointer to outputData struct to store the data
     * @return true if data retrieved successfully, false otherwise
     */
    bool getAccelData(outputData *data);

    /**
     * @brief Get raw accelerometer data (16-bit integers)
     * @param data Pointer to rawOutputData struct to store the data
     * @return true if data retrieved successfully, false otherwise
     */
    bool getRawAccelData(rawOutputData *data);

    /**
     * @brief Get the unique ID of the device
     * @return Unique ID byte
     */
    uint8_t getUniqueID();

    /**
     * @brief Get the current operating mode
     * @return Operating mode value
     */
    int8_t getOperatingMode();

private:
    SparkFun_KX134_SPI _kx134;  ///< Underlying SparkFun KX134 SPI object
    bool _initialized;          ///< Initialization status flag
};

