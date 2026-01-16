/**
 * @file main.cpp
 * @brief Main avionics system implementation
 * 
 * Implements the high-level flight control system with:
 * - Sensor polling and data aggregation
 * - Flight state machine
 * - Radio communication (uplink/downlink)
 * - Data logging (SD card + SPI flash)
 * - LED status indicators
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>
#include <stdio.h>

// Hardware libraries
#include "KX134Accelerometer.h"
#include "Radio.h"
#include "sdCard.h"
#include "dataPacket.h"

// System libraries
#include "SensorData.h"
#include "FlightState.h"

// ============================================================================
// Pin Definitions
// ============================================================================

// KX134 Accelerometer (SPI)
#define KX134_CS_PIN PB14

// Radio (RF69 - SPI)
#define RADIO_CS_PIN PA4
#define RADIO_INT_PIN PA3
#define RADIO_RST_PIN PA2

// SD Card (SPI)
#define SD_CS_PIN PA8

// SPI Flash (W25Q128 - placeholder, adjust pin as needed)
#define SPI_FLASH_CS_PIN PA15

// ============================================================================
// Global Objects
// ============================================================================

// Sensors
KX134Accelerometer accelerometer;

// Communication
Radio radio(RADIO_CS_PIN, RADIO_INT_PIN, RADIO_RST_PIN);

// Storage
sdCard card(SD_CS_PIN);

// Data structures
SensorData sensorData;
FlightStateMachine stateMachine;

// Data packet formatters for different sensor types
DataPacket accelPacket(StartByte::NO_RESPONSE);      // High-G Accelerometer
DataPacket baroPacket(StartByte::NO_RESPONSE);       // Barometer
DataPacket statusPacket(StartByte::NO_RESPONSE);    // Status checks


static constexpr uint32_t RADIO_FREQUENCY = 915000000;  // 915 MHz
static constexpr uint32_t SENSOR_READ_INTERVAL = 20;    // ms (50 Hz)
static constexpr uint32_t RADIO_TX_INTERVAL = 100;      // ms (10 Hz)
static constexpr uint32_t RADIO_RX_INTERVAL = 50;       // ms (20 Hz)

uint32_t lastSensorRead = 0;
uint32_t lastRadioTx = 0;
uint32_t lastRadioRx = 0;
uint32_t dataSequenceNumber = 0;

// ============================================================================
// Function Prototypes
// ============================================================================

void readSensors();
void updateStateMachine();
void handleRadio();
void formatAccelerometerPayload(uint8_t* payload);
void formatBarometerPayload(uint8_t* payload);
void formatStatusPayload(uint8_t* payload);
void parseRadioCommand(const DecodedPacket& decoded);
void writeLogEntry();
void writeSystemLog(const char* message);

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Wire.begin();
    // Initialize Serial
    Serial.begin(9600);
    while (!Serial && millis() < 5000) {
        delay(50);
    }
    Serial.println("\n=== Blaze Avionics System ===");
    
    // Initialize SPI
    SPI.begin();
    delay(100);
    
    // Initialize Accelerometer
    Serial.println("Initializing KX134 accelerometer...");
    SPISettings kx134Settings(1000000, MSBFIRST, SPI_MODE0);
    pinMode(KX134_CS_PIN, OUTPUT);
    digitalWrite(KX134_CS_PIN, HIGH);
    
    if (!accelerometer.begin(SPI, kx134Settings, KX134_CS_PIN)) {
        Serial.println("ERROR: KX134 initialization failed!");
        char errorMsg[128];
        snprintf(errorMsg, sizeof(errorMsg), "[%lu] ERROR: KX134 initialization failed!\r\n", millis());
        writeSystemLog(errorMsg);
        stateMachine.setError("KX134 init failed");
    } else {
        Serial.println("KX134 initialized successfully");
        accelerometer.reset();
        delay(50);
        accelerometer.enableDataEngine(true);
        accelerometer.setRange(SFE_KX134_RANGE64G);
        accelerometer.enable(true);
    }
    
    // Initialize Radio
    Serial.println("Initializing radio...");
    if (!radio.init(RADIO_FREQUENCY)) {
        Serial.println("ERROR: Radio initialization failed!");
        char errorMsg[128];
        snprintf(errorMsg, sizeof(errorMsg), "[%lu] ERROR: Radio initialization failed!\r\n", millis());
        writeSystemLog(errorMsg);
        stateMachine.setError("Radio init failed");
    } else {
        Serial.println("Radio initialized successfully");
        radio.setCallSign("BLAZE");
    }
    
    // Initialize SD Card
    Serial.println("Initializing SD card...");
    card.startUp();
    
    // Initialize State Machine
    stateMachine.init();
    Serial.println("State machine initialized - Starting in UNARMED state");
    
    // Initialize Sensor Data
    initSensorData(&sensorData);
        
    Serial.println("=== System Ready ===");
    Serial.println("Waiting for ARM command...");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    updateStateMachine();       // Flight logic
    readSensors();              // All sensor polling (includes logging)
    handleRadio();              // Uplink/downlink
}

// ============================================================================
// Sensor Reading
// ============================================================================

void readSensors() {
    uint32_t currentTime = millis();
    
    // Throttle sensor reads to avoid overwhelming the system
    if (currentTime - lastSensorRead < SENSOR_READ_INTERVAL) {
        return;
    }
    lastSensorRead = currentTime;
    
    sensorData.systemTimestamp = currentTime;
    sensorData.sequenceNumber = dataSequenceNumber++;
    
    // Read Accelerometer (KX134)
    if (accelerometer.dataReady()) {
        outputData accelData;
        if (accelerometer.getAccelData(&accelData)) {
            sensorData.accel.x = accelData.xData;
            sensorData.accel.y = accelData.yData;
            sensorData.accel.z = accelData.zData;
            sensorData.accel.magnitude = calculateAccelMagnitude(
                accelData.xData, accelData.yData, accelData.zData);
            sensorData.accel.valid = true;
            sensorData.accel.timestamp = currentTime;
        } else {
            sensorData.accel.valid = false;
        }
    } else {
        sensorData.accel.valid = false;
    }
    
    // TODO: Read Gyroscope (LSM9DS1 or similar)
    // sensorData.gyro.valid = false;  // Placeholder
    
    // TODO: Read Magnetometer (LSM9DS1 or similar)
    // sensorData.mag.valid = false;  // Placeholder
    
    // TODO: Read Barometer (BMP280/MS5611)
    // sensorData.baro.valid = false;  // Placeholder
    // For now, use a placeholder altitude based on accelerometer
    // In a real system, this would come from barometric pressure sensor
    sensorData.baro.altitude = 0.0f;  // Placeholder
    sensorData.baro.valid = false;
    
    // Log data every time sensors are read
    writeLogEntry();
}

// ============================================================================
// State Machine Update
// ============================================================================

void updateStateMachine() {
    // Get current sensor data for state machine
    float altitude = sensorData.baro.valid ? sensorData.baro.altitude : 0.0f;
    float acceleration = sensorData.accel.valid ? sensorData.accel.magnitude : 0.0f;
    
    // Update state machine
    bool stateChanged = stateMachine.update(altitude, acceleration);
    
    // Handle state changes
    if (stateChanged) {
        const FlightState& state = stateMachine.getState();
        Serial.print("State changed to: ");
        char logMsg[256];
        
        switch (state.phase) {
            case FlightPhase::UNARMED:
                Serial.println("UNARMED");
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: UNARMED\r\n", millis());
                writeSystemLog(logMsg);
                // Disable logging and radio when entering UNARMED state
                stateMachine.setLoggingEnabled(false);
                stateMachine.setRadioFlag(false);
                break;
            case FlightPhase::ARMED:
                Serial.println("ARMED");
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: ARMED\r\n", millis());
                writeSystemLog(logMsg);
                // Enable logging and radio when entering ARMED state
                stateMachine.setLoggingEnabled(true);
                stateMachine.setRadioFlag(true);
                break;
            case FlightPhase::LAUNCH:
                Serial.println("LAUNCH");
                Serial.print("Launch time: ");
                Serial.println(state.launchTime);
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: LAUNCH (time: %lu)\r\n", millis(), state.launchTime);
                writeSystemLog(logMsg);
                break;
            case FlightPhase::APOGEE:
                Serial.println("APOGEE");
                Serial.print("Max altitude: ");
                Serial.print(state.maxAltitude);
                Serial.println(" m");
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: APOGEE (max alt: %.2f m)\r\n", millis(), state.maxAltitude);
                writeSystemLog(logMsg);
                break;
            case FlightPhase::DESCENT:
                Serial.println("DESCENT");
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: DESCENT\r\n", millis());
                writeSystemLog(logMsg);
                break;
            case FlightPhase::LANDED:
                Serial.println("LANDED");
                Serial.print("Landed time: ");
                Serial.println(state.landedTime);
                snprintf(logMsg, sizeof(logMsg), "[%lu] STATE: LANDED (time: %lu)\r\n", millis(), state.landedTime);
                writeSystemLog(logMsg);
                break;
            case FlightPhase::ERROR:
                Serial.println("ERROR");
                Serial.print("Error: ");
                Serial.println(state.errorMessage);
                snprintf(logMsg, sizeof(logMsg), "[%lu] ERROR: %s\r\n", millis(), state.errorMessage);
                writeSystemLog(logMsg);
                break;
        }
    }
    
    // Check for ARM command (could be from radio or button)
}

// ============================================================================
// Radio Handling
// ============================================================================

void handleRadio() {
    uint32_t currentTime = millis();
    const FlightState& state = stateMachine.getState();
    
    // Receive radio commands during UNARMED and ARMED phases
    // Check for incoming messages every 50ms
    if ((currentTime - lastRadioRx >= RADIO_RX_INTERVAL)) {
        lastRadioRx = currentTime;
        
        // Check for incoming radio messages (uplink)
        if (radio.available()) {
            uint8_t rxBuffer[64];
            size_t received = radio.recv(rxBuffer, sizeof(rxBuffer));
            
            if (received > 0) {
                // Try to decode as a DataPacket
                DecodedPacket decoded;
                if (received == DataPacket::PACKET_SIZE) {
                    DataPacket tempPacket(StartByte::NO_RESPONSE);
                    if (tempPacket.decodePacket(rxBuffer, received, decoded)) {
                        // Log received telemetry/command
                        char logMsg[256];
                        snprintf(logMsg, sizeof(logMsg), 
                            "[%lu] RX: ID=%c%c, Seq=%lu, TS=%lu\r\n", 
                            millis(), decoded.idA, decoded.idB, decoded.sequenceID, decoded.timestamp);
                        writeSystemLog(logMsg);
                        
                        parseRadioCommand(decoded);
                    } else {
                        Serial.println("Failed to decode packet");
                        char logMsg[128];
                        snprintf(logMsg, sizeof(logMsg), "[%lu] ERROR: Failed to decode packet\r\n", millis());
                        writeSystemLog(logMsg);
                    }
                } else {
                    Serial.print("Invalid packet size: ");
                    Serial.println(received);
                    char logMsg[128];
                    snprintf(logMsg, sizeof(logMsg), "[%lu] ERROR: Invalid packet size: %u\r\n", millis(), received);
                    writeSystemLog(logMsg);
                }
            }
        }
    }
    
    // Send telemetry (downlink) at regular intervals
    // According to protocol: send sensor-specific packets
    if (state.radioFlag && (currentTime - lastRadioTx >= RADIO_TX_INTERVAL)) {
        lastRadioTx = currentTime;
        
        // Send High-G Accelerometer data (ID: "al")
        if (sensorData.accel.valid) {
            uint8_t payload[DataPacket::PAYLOAD_SIZE];
            formatAccelerometerPayload(payload);
            accelPacket.encodePacket(payload, 'a', 'l');
            
            uint8_t* packetBuffer = accelPacket.getBuffer();
            size_t packetSize = accelPacket.getLength();
            radio.send(packetBuffer, packetSize, false);
        }
        
        // Send Barometer data (ID: "ba") if available
        if (sensorData.baro.valid) {
            uint8_t payload[DataPacket::PAYLOAD_SIZE];
            formatBarometerPayload(payload);
            baroPacket.encodePacket(payload, 'b', 'a');
            
            uint8_t* packetBuffer = baroPacket.getBuffer();
            size_t packetSize = baroPacket.getLength();
            radio.send(packetBuffer, packetSize, false);
        }
        
        // Send Status checks (ID: "sc")
        {
            uint8_t payload[DataPacket::PAYLOAD_SIZE];
            formatStatusPayload(payload);
            statusPacket.encodePacket(payload, 's', 'c');
            
            uint8_t* packetBuffer = statusPacket.getBuffer();
            size_t packetSize = statusPacket.getLength();
            radio.send(packetBuffer, packetSize, false);
        }
    }
}

// ============================================================================
// Logging
// ============================================================================

void writeLogEntry() {
    const FlightState& state = stateMachine.getState();
    
    // Only log if logging is enabled
    if (!state.loggingEnabled) {
        return;
    }
    
    // Format data for logging
    char logBuffer[256];
    snprintf(logBuffer, sizeof(logBuffer),
        "%lu,%u,%.3f,%.3f,%.3f,%.3f,%.2f,%u\r\n",
        sensorData.systemTimestamp,
        sensorData.sequenceNumber,
        sensorData.accel.x,
        sensorData.accel.y,
        sensorData.accel.z,
        sensorData.accel.magnitude,
        sensorData.baro.altitude,
        static_cast<uint8_t>(state.phase)
    );
    
    // Write to SD card
    ssize_t written = card.writeData(strlen(logBuffer), logBuffer);
    if (written < 0) {
        Serial.println("SD write failed");
        char logMsg[128];
        snprintf(logMsg, sizeof(logMsg), "[%lu] ERROR: SD data write failed\r\n", millis());
        writeSystemLog(logMsg);
    }
    
    // TODO: Write to SPI Flash (W25Q128)
    // This would require a SPI flash library
    // For now, this is a placeholder
}

/**
 * Write system log entry to separate log file (Log.txt)
 * Used for errors, state changes, and received telemetry/commands
 */
void writeSystemLog(const char* message) {
    if (message == nullptr) {
        return;
    }
    
    // Write to log file using writeLog method
    ssize_t written = card.writeLog(message, strlen(message));
    if (written < 0) {
        // If log write fails, at least try to print to Serial
        Serial.print("Log write failed: ");
        Serial.println(message);
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Format High-G Accelerometer payload (ID: "al")
 * Payload: acceleration-x, acceleration-y, acceleration-z
 * Format: ASCII string with leading zeros, 17 bytes total
 * Example: "00000546789764783" (X=5467, Y=8976, Z=4783 with leading zeros)
 */
void formatAccelerometerPayload(uint8_t* payload) {
    if (payload == nullptr) {
        return;
    }
    
    // Convert accelerometer values to integers (multiply by 1000 for precision)
    // Then format as ASCII strings with leading zeros
    int32_t accelX = (int32_t)(sensorData.accel.x * 1000.0f);
    int32_t accelY = (int32_t)(sensorData.accel.y * 1000.0f);
    int32_t accelZ = (int32_t)(sensorData.accel.z * 1000.0f);
    
    // Format: X(5 digits) + Y(6 digits) + Z(6 digits) = 17 bytes
    // Using format: "XXXXXYYYYYYZZZZZZ"
    char temp[18];
    snprintf(temp, sizeof(temp), "%05ld%06ld%06ld", 
             (long)accelX, (long)accelY, (long)accelZ);
    
    // Copy to payload (ensure exactly 17 bytes)
    for (int i = 0; i < 17; i++) {
        payload[i] = (i < strlen(temp)) ? temp[i] : '0';
    }
}

/**
 * Format Barometer payload (ID: "ba")
 * Payload: pressure value
 * Format: ASCII string with leading zeros, 17 bytes total
 * Example: "00000000000000546" (pressure value with leading zeros)
 */
void formatBarometerPayload(uint8_t* payload) {
    if (payload == nullptr) {
        return;
    }
    
    // Convert pressure to integer (Pa, multiply by 1 for integer representation)
    // Format as ASCII string with leading zeros
    int32_t pressure = (int32_t)sensorData.baro.pressure;
    
    char temp[18];
    snprintf(temp, sizeof(temp), "%017ld", (long)pressure);
    
    // Copy to payload (ensure exactly 17 bytes)
    for (int i = 0; i < 17; i++) {
        payload[i] = (i < strlen(temp)) ? temp[i] : '0';
    }
}

/**
 * Format Status checks payload (ID: "sc")
 * Payload: flight state and system status
 * Format: ASCII string with leading zeros, 17 bytes total
 */
void formatStatusPayload(uint8_t* payload) {
    if (payload == nullptr) {
        return;
    }
    
    const FlightState& state = stateMachine.getState();
    
    // Format: phase(1) + altitude(8) + maxAltitude(8) = 17 bytes
    // Example: "10000000000000000" (phase=1, altitude=0, maxAltitude=0)
    char temp[18];
    int32_t altitude = (int32_t)(state.altitude * 100.0f);  // cm precision
    int32_t maxAltitude = (int32_t)(state.maxAltitude * 100.0f);
    
    snprintf(temp, sizeof(temp), "%01d%08ld%08ld", 
             static_cast<int>(state.phase), (long)altitude, (long)maxAltitude);
    
    // Copy to payload (ensure exactly 17 bytes)
    for (int i = 0; i < 17; i++) {
        payload[i] = (i < strlen(temp)) ? temp[i] : '0';
    }
}

/**
 * Parse incoming radio command using decoded packet
 * Command IDs (Ground → Air):
 * - "ss" (0x7373): System command (reboot, etc.)
 * - "sm" (0x736D): State machine command (ARM/DISARM)
 * - "pr" (0x7072): Ping request (status checks)
 */
void parseRadioCommand(const DecodedPacket& decoded) {
    if (!decoded.isValid) {
        Serial.println("Invalid packet received");
        char logMsg[128];
        snprintf(logMsg, sizeof(logMsg), "[%lu] ERROR: Invalid packet received\r\n", millis());
        writeSystemLog(logMsg);
        return;
    }
    
    // Extract message ID
    char idA = decoded.idA;
    char idB = decoded.idB;
    char logMsg[256];
    
    // Check command type based on message ID
    if (idA == 's' && idB == 's') {
        // System command (ss)
        Serial.println("System command received");
        snprintf(logMsg, sizeof(logMsg), "[%lu] CMD: System command (ss) received\r\n", millis());
        writeSystemLog(logMsg);
        // Payload could contain reboot command, etc.
        // For now, just acknowledge
        Serial.println("System command acknowledged");
        
    } else if (idA == 's' && idB == 'm') {
        // State machine command (sm) - ARM/DISARM
        Serial.println("State machine command received");
        snprintf(logMsg, sizeof(logMsg), "[%lu] CMD: State machine command (sm) received\r\n", millis());
        writeSystemLog(logMsg);
        
        // Parse payload for state command
        // Assuming payload[0] contains command: 0=DISARM, 1=ARM
        if (decoded.payload[0] == '1' || decoded.payload[0] == 1) {
            Serial.println("ARM command");
            snprintf(logMsg, sizeof(logMsg), "[%lu] CMD: ARM command executed\r\n", millis());
            writeSystemLog(logMsg);
            stateMachine.setPhase(FlightPhase::ARMED);
        } else if (decoded.payload[0] == '0' || decoded.payload[0] == 0) {
            Serial.println("DISARM command");
            snprintf(logMsg, sizeof(logMsg), "[%lu] CMD: DISARM command executed\r\n", millis());
            writeSystemLog(logMsg);
            stateMachine.setPhase(FlightPhase::UNARMED);
        }
        
    } else if (idA == 'p' && idB == 'r') {
        // Ping request (pr) - status checks
        Serial.println("Ping request received");
        snprintf(logMsg, sizeof(logMsg), "[%lu] CMD: Ping request (pr) received\r\n", millis());
        writeSystemLog(logMsg);
        // Send a ping response packet
        uint8_t payload[DataPacket::PAYLOAD_SIZE];
        const FlightState& state = stateMachine.getState();
        // Format ping response: "PingResp" + phase number + status
        // Format: "PingResp" (8 chars) + phase (1 char) + "OK" (2 chars) + padding = 17 bytes
        snprintf((char*)payload, sizeof(payload), "PingResp%01dOK      ", static_cast<int>(state.phase));
        statusPacket.encodePacket(payload, 'p', 'r');
        
        // Send the ping response packet
        uint8_t* packetBuffer = statusPacket.getBuffer();
        size_t packetSize = statusPacket.getLength();
        radio.send(packetBuffer, packetSize, false);
    } else {
        Serial.print("Unknown command ID: ");
        Serial.print(idA);
        Serial.println(idB);
        snprintf(logMsg, sizeof(logMsg), "[%lu] WARN: Unknown command ID: %c%c\r\n", millis(), idA, idB);
        writeSystemLog(logMsg);
    }
    
    // Log received packet info
    Serial.print("Sequence: ");
    Serial.print(decoded.sequenceID);
    Serial.print(", Timestamp: ");
    Serial.println(decoded.timestamp);
}
