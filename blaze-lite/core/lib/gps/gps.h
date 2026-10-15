#pragma once
#include <Arduino.h>
// #include <SPI.h> (investigate)


// tune if necessary during sims

struct gpsData{
    int32_t latitudeE7; // latitude in degrees, shifted by 1e7
    int32_t longitudeE7;
    float altitude;
    uint32_t lastValidPositionTimestamp;

    float speed3D; // full 3D speed
    float speed2D; // horizontal speed relative to ground
    float velNorth, velEast, velDown;
    uint32_t lastValidSpeedTimestamp;

    bool validity;
    uint32_t timestampMs; // RECEIVER's net timestamp in milliseconds. not necessarily a valid one for all fields.
    uint8_t satelliteCount;
    
    float accuracyHorizontal;
    float accuracyVertical;
    float accuracySpeed;
}; // not comprehensive, we can add more fields as necessary


class GPS {
    private:
        gpsData data = {};
        bool isInitialized = false;
        bool isConnected = false;

        const uint32_t TIMEOUT_TOLERANCE_MS = 5000; // this is probably way too high.

        // initialize all hardware-facing consts like ports/pins here. there might also be need for some helpers
        bool setConnectState(bool state); 
    public:
        GPS();
        /** Sets up and configures the connection. Also updates the isInitialized flag.
         * @returns true if successful, false otherwise.
         */
        bool init();

        /** Getter for connected state */
        bool getConnectState(); // returns the connection state

        /** Reads the packet and updates all fields. */
        void update(); 

        /** Returns the current GPS data. */
        gpsData getData() const { return data; }
};

