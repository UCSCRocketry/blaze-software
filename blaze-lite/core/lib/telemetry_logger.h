#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <Arduino.h>
#include <SD.h>
#include "barometer.h"
#include "sdcard_class"  // Assuming this is available

class TelemetryLogger {
    public:
        TelemetryLogger(const char* filename, uint32_t flushInterval = 50);
        
        // Initialize logger and create/open file
        bool init();
        
        // Log a barometer reading to file
        bool logReading(const BarometerReading& reading);
        
        // Manually flush buffered data to SD card
        bool flush();
        
        // Close the current file
        void close();
        
        // Get total readings logged
        uint32_t getReadingCount() const;
        
        // Set write priority (use sdCard priority constants)
        void setWritePriority(char priority);
        
    private:
        const char* filename;
        File dataFile;
        uint32_t readingCount;
        uint32_t bufferCount;
        uint32_t flushInterval;
        char writePriority;
        bool headerWritten;
        
        // Internal helper to format and write CSV line
        bool writeCSVLine(const BarometerReading& reading);
};

#endif // TELEMETRY_LOGGER_H
