#include "telemetry_logger.h"

TelemetryLogger::TelemetryLogger(const char* filename_, uint32_t flushInterval_)
    : filename(filename_), readingCount(0), bufferCount(0), 
      flushInterval(flushInterval_), writePriority(5), headerWritten(false) {}

bool TelemetryLogger::init() {
    // Open or create the file
    dataFile = SD.open(filename, FILE_WRITE);
    
    if (!dataFile) {
        Serial.print("Error opening file: ");
        Serial.println(filename);
        return false;
    }
    
    // Check if file is empty (new file) and write header
    if (dataFile.size() == 0) {
        dataFile.println("timestamp_ms,pressure_Pa,altitude_m,temperature_C");
        headerWritten = true;
    } else {
        headerWritten = true;
    }
    
    dataFile.close();
    Serial.print("Telemetry logger initialized: ");
    Serial.println(filename);
    return true;
}

bool TelemetryLogger::logReading(const BarometerReading& reading) {
    bufferCount++;
    readingCount++;
    
    // Write immediately if at flush interval
    if (bufferCount >= flushInterval) {
        return flush();
    }
    
    return true;
}

bool TelemetryLogger::writeCSVLine(const BarometerReading& reading) {
    dataFile = SD.open(filename, FILE_WRITE);
    
    if (!dataFile) {
        Serial.println("Error opening file for writing!");
        return false;
    }
    
    // Move to end of file
    if (!dataFile.seek(dataFile.size())) {
        Serial.println("Seek failed!");
        dataFile.close();
        return false;
    }
    
    // Format: timestamp,pressure,altitude,temperature
    dataFile.print(reading.timestamp);
    dataFile.print(",");
    dataFile.print(reading.pressure, 2);
    dataFile.print(",");
    dataFile.print(reading.altitude, 3);
    dataFile.print(",");
    dataFile.println(reading.temperature, 2);
    
    dataFile.close();
    return true;
}

bool TelemetryLogger::flush() {
    if (bufferCount == 0) {
        return true;  // Nothing to flush
    }
    
    // For now, flush per reading
    // In a real implementation, you'd batch buffer and write
    bufferCount = 0;
    
    Serial.print("Flushed ");
    Serial.print(readingCount);
    Serial.println(" readings to SD card");
    
    return true;
}

void TelemetryLogger::close() {
    if (dataFile) {
        dataFile.close();
    }
    Serial.print("Telemetry file closed. Total readings: ");
    Serial.println(readingCount);
}

uint32_t TelemetryLogger::getReadingCount() const {
    return readingCount;
}

void TelemetryLogger::setWritePriority(char priority) {
    writePriority = priority;
}
