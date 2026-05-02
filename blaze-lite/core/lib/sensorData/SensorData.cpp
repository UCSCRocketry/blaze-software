/**
 * @file SensorData.cpp
 * @brief Implementation of SensorData utilities
 */

#include "SensorData.h"
#include <math.h>
#include <string.h>

void initSensorData(SensorData* data) {
    if (data == nullptr) {
        return;
    }
    
    memset(data, 0, sizeof(SensorData));
    
    data->accel.x = 0.0f;
    data->accel.y = 0.0f;
    data->accel.z = 0.0f;
    data->accel.magnitude = 0.0f;
    data->accel.valid = false;
    data->accel.timestamp = 0;
    
    /*
    data->gyro.x = 0.0f;
    data->gyro.y = 0.0f;
    data->gyro.z = 0.0f;
    data->gyro.valid = false;
    data->gyro.timestamp = 0;
    
    data->mag.x = 0.0f;
    data->mag.y = 0.0f;
    data->mag.z = 0.0f;
    data->mag.valid = false;
    data->mag.timestamp = 0;
    */
    
    data->baro.pressure = 0.0f;
    data->baro.altitude = 0.0f;
    data->baro.temperature = 0.0f;
    data->baro.valid = false;
    data->baro.timestamp = 0;
    
    data->systemTimestamp = 0;
    data->sequenceNumber = 0;
}

float calculateAccelMagnitude(float x, float y, float z) {
    return sqrtf(x * x + y * y + z * z);
}

void printSensorData(const SensorData& data) {
    Serial.print("SensorData [Seq: "); Serial.print(data.sequenceNumber);
    Serial.print("] Accel(V:"); Serial.print(data.accel.valid);
    Serial.print(") X:"); Serial.print(data.accel.x, 3);
    Serial.print(" Y:"); Serial.print(data.accel.y, 3);
    Serial.print(" Z:"); Serial.print(data.accel.z, 3);
    Serial.print(" Mag:"); Serial.print(data.accel.magnitude, 3);
    Serial.print(" | Baro(V:"); Serial.print(data.baro.valid);
    Serial.print(") P:"); Serial.print(data.baro.pressure, 2);
    Serial.print(" T:"); Serial.print(data.baro.temperature, 2);
    Serial.print(" Alt:"); Serial.println(data.baro.altitude, 2);
}
