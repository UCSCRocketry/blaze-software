/**
 * @file SensorData.h
 * @brief Shared data structures for sensor readings
 * 
 * Centralized data structure for all sensor data used throughout the system
 */

#pragma once

#include <Arduino.h>
#include <SparkFun_KX13X.h>

/**
 * @struct SensorData
 * @brief Aggregated sensor data from all sensors
 */
struct SensorData {
    // Accelerometer data (KX134)
    struct {
        float x;        ///< X-axis acceleration (g)
        float y;        ///< Y-axis acceleration (g)
        float z;        ///< Z-axis acceleration (g)
        float magnitude; ///< Acceleration magnitude (g)
        bool valid;     ///< Data validity flag
        uint32_t timestamp; ///< Timestamp of reading (ms)
    } accel;
    
    // Barometric pressure data (BMP280/MS5611 - placeholder for future implementation)
    struct {
        float pressure;     ///< Pressure (Pa)
        float altitude;     ///< Calculated altitude (m)
        float temperature;  ///< Temperature (C)
        bool valid;         ///< Data validity flag
        uint32_t timestamp; ///< Timestamp of reading (ms)
    } baro;
    
    // System metadata
    uint32_t systemTimestamp;  ///< System timestamp when data was collected (ms)
    uint32_t sequenceNumber;   ///< Sequence number for this data packet
};

/**
 * @brief Initialize SensorData structure with default values
 * @param data Pointer to SensorData structure to initialize
 */
void initSensorData(SensorData* data);

/**
 * @brief Calculate acceleration magnitude from x, y, z components
 * @param x X-axis acceleration
 * @param y Y-axis acceleration
 * @param z Z-axis acceleration
 * @return Magnitude of acceleration
 */
float calculateAccelMagnitude(float x, float y, float z);
