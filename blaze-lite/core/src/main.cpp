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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Hardware libraries
#include "KX134Accelerometer.h"
#include "Radio.h"
#include "sdCard.h"
#include "spiFlash.h"
#include "dataPacket.h"

// System libraries
#include "SensorData.h"
#include "FlightState.h"
#include "Baro.h"

// ============================================================================
// Pin Definitions
// ============================================================================


// Radio (RF69 - SPI)
#define RADIO_CS_PIN PA4
#define RADIO_INT_PIN PA3
#define RADIO_RST_PIN PB10

// SD Card (SPI)
#define SD_CS_PIN PB15

// SPI Flash (W25Q128 - placeholder, adjust pin as needed)
#define SPI_FLASH_CS_PIN PB8

#define BARO_CS_PIN PB9
 

// ============================================================================
// SPI Settings
// ============================================================================

#define SPI_SCK_PIN  PA5
#define SPI_MISO_PIN PA6
#define SPI_MOSI_PIN PA7

Baro barometer(BARO_CS_PIN); 

// ============================================================================
// Global Objects
// ============================================================================

// Sensors
KX134Accelerometer accelerometer;

// Communication
Radio radio(RADIO_CS_PIN, RADIO_INT_PIN, RADIO_RST_PIN);

// Storage
sdCard card(SD_CS_PIN);
spiFlash spiFlashMem;
bool spiFlashReady = false;

// Data structures
SensorData sensorData;
FlightStateMachine stateMachine;

// Data packet formatters for different sensor types
DataPacket accelPacket(StartByte::NO_RESPONSE);      // High-G Accelerometer
DataPacket baroPacket(StartByte::NO_RESPONSE);       // Barometer
DataPacket statusPacket(StartByte::NO_RESPONSE);    // Status checks


static constexpr uint32_t RADIO_FREQUENCY = 433;  // 433 MHz
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
bool formatAccelerometerPayload(uint8_t* payload);
bool formatBarometerPayload(uint8_t* payload);
bool formatStatusPayload(uint8_t* payload);
void parseRadioCommand(const DecodedPacket& decoded);
void writeLogEntry();
void writeSystemLog(const char* format, ...);
void printReceivedPacket(const uint8_t* buffer, size_t length, const DecodedPacket* decoded);
// ============================================================================
// Setup
// ============================================================================

void setup() {
#if defined(ARDUINO_BLAZE_F411CE)
    // Status LEDs: PC13 is the separate “Arduino” LED; blue channel is PA10 RGB.
    // If blue stays off, set build flag -D BLAZE_LED_RGB_ON=LOW for common-anode RGB.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);
    pinMode(LED_RGB_R, OUTPUT);
    pinMode(LED_RGB_G, OUTPUT);
    pinMode(LED_RGB_B, OUTPUT);
    digitalWrite(LED_RGB_R, BLAZE_LED_RGB_OFF);
    digitalWrite(LED_RGB_G, BLAZE_LED_RGB_OFF);
    digitalWrite(LED_RGB_B, BLAZE_LED_RGB_ON);

    //Write all CS pins to high
    pinMode(RADIO_CS_PIN, OUTPUT);
    digitalWrite(RADIO_CS_PIN, HIGH);
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    pinMode(SPI_FLASH_CS_PIN, OUTPUT);
    digitalWrite(SPI_FLASH_CS_PIN, HIGH);
    pinMode(PB2, OUTPUT); //Accelerometer CS pin
    digitalWrite(PB2, HIGH);
    pinMode(PA0, OUTPUT);
    digitalWrite(PA0, HIGH);
    pinMode(PA1, OUTPUT);
    digitalWrite(PA1, HIGH);
    pinMode(PA2, OUTPUT);
    digitalWrite(PA2, HIGH);
    pinMode(PB9, OUTPUT);
    digitalWrite(PB9, HIGH);

#endif

    // Initialize Serial
    Serial.begin(9600);
    while (!Serial && millis() < 5000) {
        delay(50);
    }
    Serial.println("\n=== Blaze Avionics System ===");

    Serial.println("Debug: Setting SPI pins...");
    // Initialize SPI on explicit SPI1 pins
    SPI.setSCLK(SPI_SCK_PIN);
    SPI.setMISO(SPI_MISO_PIN);
    SPI.setMOSI(SPI_MOSI_PIN);

    Serial.println("Debug: Initializing SPI bus...");

    SPI.begin();
    delay(2000);
    
    // Initialize Radio
    Serial.println("Initializing radio...");
    if (!radio.init(RADIO_FREQUENCY)) {
        writeSystemLog("[%lu] ERROR: Radio initialization failed!\r\n", millis());
        stateMachine.setError("Radio init failed");
    } else {
        Serial.println("Radio initialized successfully");
        radio.setCallSign("KO6JIZ");
    }

    // // Initialize Accelerometer
    Serial.println("Initializing KX134 accelerometer...");
    if (!accelerometer.begin(SPI)) {
        writeSystemLog("[%lu] ERROR: KX134 initialization failed!\r\n", millis());
        stateMachine.setError("KX134 init failed");
    } else {
        Serial.println("KX134 initialized successfully");
        accelerometer.reset();
        delay(50);
        accelerometer.enableDataEngine(true);
        accelerometer.setRange(SFE_KX134_RANGE64G);
        accelerometer.enable(true);
    }
    
    // Initialize SD Card
    Serial.println("Initializing SD card...");
    card.startUp();

    Serial.println("Initializing SPI flash...");
    spiFlashReady = spiFlashMem.startUp();
    if (!spiFlashReady) {
        Serial.println("SPI flash unavailable (logging to SD only)");
    } else {
        Serial.println("SPI flash initialized successfully");
        //TODO: unmounted due to testing only. 
        spiFlashMem.unmountfs();
    }

    if (barometer.init() == true)
    {
        Serial.print("MS5611 found: ");
        Serial.println(barometer.getDeviceID(), HEX);
    }
    else
    {
        Serial.println("MS5611 not found. halt.");
    }
    

    // Initialize State Machine
    stateMachine.init();
    Serial.println("State machine initialized - Starting in UNARMED state");
    
    // Initialize Sensor Data
    initSensorData(&sensorData);

    stateMachine.setPhase(FlightPhase::UNARMED);
        
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
    if (spiFlashReady) {
        spiFlashMem.tick();
    }
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
    
    // Read Barometer (MS5611)
    if (barometer.isReady()) {
        barometer.read();
        sensorData.baro.pressure = barometer.getPressure();
        sensorData.baro.temperature = barometer.getTemperature();
        sensorData.baro.altitude = barometer.getAltitude();
        sensorData.baro.valid = true;
        sensorData.baro.timestamp = currentTime;
    } else {
        sensorData.baro.valid = false;
    }

    //printSensorData(sensorData);
    
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
                writeSystemLog("[%lu] STATE: UNARMED\r\n", millis());
                // Disable logging and radio when entering UNARMED state
                stateMachine.setLoggingEnabled(false);
                stateMachine.setRadioFlag(false);
                //if the fs is mounted, unmount it to ensure data integrity
                if (spiFlashMem.isMounted()) {
                    spiFlashMem.unmountfs();
                }
                break;
            case FlightPhase::ARMED:
                writeSystemLog("[%lu] STATE: ARMED\r\n", millis());
                // Enable logging and radio when entering ARMED state
                stateMachine.setLoggingEnabled(true);
                stateMachine.setRadioFlag(true);
                spiFlashMem.mountfs();
                break;
            case FlightPhase::LAUNCH:
                writeSystemLog("[%lu] STATE: LAUNCH (time: %lu)\r\n", millis(), state.launchTime);
                break;
            case FlightPhase::APOGEE:
                writeSystemLog("[%lu] STATE: APOGEE (max alt: %.2f m)\r\n", millis(), state.maxAltitude);
                break;
            case FlightPhase::DESCENT:
                writeSystemLog("[%lu] STATE: DESCENT\r\n", millis());
                break;
            case FlightPhase::LANDED:
                writeSystemLog("[%lu] STATE: LANDED (time: %lu)\r\n", millis(), state.landedTime);
                break;
            case FlightPhase::ERROR:
                writeSystemLog("[%lu] ERROR: %s\r\n", millis(), state.errorMessage);
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
                        writeSystemLog("[%lu] RX: ID=%c%c, Seq=%lu, TS=%lu\r\n", 
                            millis(), decoded.idA, decoded.idB, decoded.sequenceID, decoded.timestamp);

                        // Print full packet details to Serial for debugging
                        printReceivedPacket(rxBuffer, received, &decoded);
                        
                        parseRadioCommand(decoded);
                    } else {
                        writeSystemLog("[%lu] ERROR: Failed to decode packet\r\n", millis());
                    }
                } else {
                    // Debug-only: print raw bytes for non-DataPacket lengths
                    printReceivedPacket(rxBuffer, received, nullptr);
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
            if (!formatAccelerometerPayload(payload)) {
                writeSystemLog("[%lu] ERROR: Failed to format accelerometer payload\r\n", millis());
                return;
            }
            accelPacket.encodePacket(payload, 'a', 'l');
            
            uint8_t* packetBuffer = accelPacket.getBuffer();
            size_t packetSize = accelPacket.getLength();
            radio.send(packetBuffer, packetSize, false);
        }
        
        // Send Barometer data (ID: "ba") if available
        if (sensorData.baro.valid) {
            uint8_t payload[DataPacket::PAYLOAD_SIZE];
            if (!formatBarometerPayload(payload)) {
                writeSystemLog("[%lu] ERROR: Failed to format barometer payload\r\n", millis());
                return;
            }
            baroPacket.encodePacket(payload, 'b', 'a');
            
            uint8_t* packetBuffer = baroPacket.getBuffer();
            size_t packetSize = baroPacket.getLength();
            radio.send(packetBuffer, packetSize, false);
        }
        
        // Send Status checks (ID: "sc")
        {
            uint8_t payload[DataPacket::PAYLOAD_SIZE];
            if (!formatStatusPayload(payload)) {
                writeSystemLog("[%lu] ERROR: Failed to format status payload\r\n", millis());
                return;
            }
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
    
    float accelX = sensorData.accel.valid ? sensorData.accel.x : 0.0f;
    float accelY = sensorData.accel.valid ? sensorData.accel.y : 0.0f;
    float accelZ = sensorData.accel.valid ? sensorData.accel.z : 0.0f;
    float accelMag = sensorData.accel.valid ? sensorData.accel.magnitude : 0.0f;
    float baroAlt = sensorData.baro.valid ? sensorData.baro.altitude : 0.0f;

    char accelXStr[16];
    char accelYStr[16];
    char accelZStr[16];
    char accelMagStr[16];
    char baroAltStr[16];

    dtostrf(accelX, 0, 3, accelXStr);
    dtostrf(accelY, 0, 3, accelYStr);
    dtostrf(accelZ, 0, 3, accelZStr);
    dtostrf(accelMag, 0, 3, accelMagStr);
    dtostrf(baroAlt, 0, 2, baroAltStr);

    // Format data for logging
    char logBuffer[256];
    snprintf(logBuffer, sizeof(logBuffer),
        "%lu,%u,%s,%s,%s,%s,%s,%u\r\n",
        sensorData.systemTimestamp,
        sensorData.sequenceNumber,
        accelXStr,
        accelYStr,
        accelZStr,
        accelMagStr,
        baroAltStr,
        static_cast<uint8_t>(state.phase)
    );
    
    // Write to SD card
    ssize_t written = card.writeData(strlen(logBuffer), logBuffer);
    if (written < 0) {
        writeSystemLog("[%lu] ERROR: SD data write failed\r\n", millis());
    }

    if (spiFlashReady) {
        if (spiFlashMem.queue(strlen(logBuffer), logBuffer, spiFlash::P_STD) < 0) {
            Serial.println("SPI flash queue failed");
        }
    }
}

/**
 * Write system log entry to separate log file (Log.txt)
 * Used for errors, state changes, and received telemetry/commands
 */
void writeSystemLog(const char* format, ...) {
    if (format == nullptr) {
        return;
    }
    Serial.println(format);
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    Serial.print(message);
    
    // Write to log file using writeLog method
    ssize_t written = card.writeLog(message, strlen(message));
    if (written < 0) {
        // If log write fails, at least try to print to Serial
        Serial.print("Log write failed: ");
        Serial.println(message);
    }

    if (spiFlashReady) {
        size_t len = strlen(message);
        if (spiFlashMem.kLog(len, message) < 0 || spiFlashMem.kflush() != 0) {
            Serial.println("SPI flash log write failed");
        }
    }
}

/**
 * Print received packet details to Serial.
 * Includes raw bytes and decoded fields when available.
 */
void printReceivedPacket(const uint8_t* buffer, size_t length, const DecodedPacket* decoded) {
    if (buffer == nullptr || length == 0) {
        Serial.println("RX: <empty>");
        return;
    }

    Serial.print("RX RAW [");
    Serial.print(length);
    Serial.print("B]: ");
    for (size_t i = 0; i < length; i++) {
        if (buffer[i] < 0x10) {
            Serial.print('0');
        }
        Serial.print(buffer[i], HEX);
        if (i + 1 < length) {
            Serial.print(' ');
        }
    }
    Serial.println();

    if (decoded != nullptr && decoded->isValid) {
        Serial.print("RX DEC: ID=");
        Serial.print(decoded->idA);
        Serial.print(decoded->idB);
        Serial.print(", Seq=");
        Serial.print(decoded->sequenceID);
        Serial.print(", TS=");
        Serial.print(decoded->timestamp);
        Serial.print(", Payload=\"");
        for (size_t i = 0; i < DataPacket::PAYLOAD_SIZE; i++) {
            char c = static_cast<char>(decoded->payload[i]);
            if (c >= 32 && c <= 126) {
                Serial.print(c);
            } else {
                Serial.print('.');
            }
        }
        Serial.println("\"");
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
bool formatAccelerometerPayload(uint8_t* payload) {
    if (payload == nullptr) {
        writeSystemLog("[%lu] ERROR: Invalid packet received\r\n", millis());
        return false;
    }

    float accelXVal = sensorData.accel.valid ? sensorData.accel.x : 0.0f;
    float accelYVal = sensorData.accel.valid ? sensorData.accel.y : 0.0f;
    float accelZVal = sensorData.accel.valid ? sensorData.accel.z : 0.0f;

    // Convert accelerometer values to integers (multiply by 1000 for precision)
    // Then format as ASCII strings with leading zeros
    int32_t accelX = (int32_t)(accelXVal * 1000.0f);
    int32_t accelY = (int32_t)(accelYVal * 1000.0f);
    int32_t accelZ = (int32_t)(accelZVal * 1000.0f);
    
    // Format: X(5 digits) + Y(6 digits) + Z(6 digits) = 17 bytes
    // Using format: "XXXXXYYYYYYZZZZZZ"
    char temp[18];
    snprintf(temp, sizeof(temp), "%05ld%06ld%06ld", 
             (long)accelX, (long)accelY, (long)accelZ);
    
    // Copy to payload (ensure exactly 17 bytes)
    for (int i = 0; i < 17; i++) {
        payload[i] = (i < strlen(temp)) ? temp[i] : '0';
    }
    return true;
}

/**
 * Format Barometer payload (ID: "ba")
 * Payload: pressure value
 * Format: ASCII string with leading zeros, 17 bytes total
 * Example: "00000000000000546" (pressure value with leading zeros)
 */
bool formatBarometerPayload(uint8_t* payload) {
    if (payload == nullptr) {
        return false;
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
    return true;
}

/**
 * Format Status checks payload (ID: "sc")
 * Payload: flight state and system status
 * Format: ASCII string with leading zeros, 17 bytes total
 */
bool formatStatusPayload(uint8_t* payload) {
    if (payload == nullptr) {
        return false;
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
    return true;
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
        writeSystemLog("[%lu] ERROR: Invalid packet received\r\n", millis());
        return;
    }
    
    // Extract message ID
    char idA = decoded.idA;
    char idB = decoded.idB;
    
    // Check command type based on message ID
    if (idA == 's' && idB == 's') {
        // System command (ss)
        writeSystemLog("[%lu] CMD: System command (ss) received\r\n", millis());
        // Payload could contain reboot command, etc.
        // For now, just acknowledge
        Serial.println("System command acknowledged");
        
    } else if (idA == 's' && idB == 'm') {
        // State machine command (sm) - ARM/DISARM
        writeSystemLog("[%lu] CMD: State machine command (sm) received\r\n", millis());
        
        // Parse payload for state command
        // Assuming payload[0] contains command: 0=DISARM, 1=ARM
        if (decoded.payload[0] == '1' || decoded.payload[0] == 1) {
            writeSystemLog("[%lu] CMD: ARM command executed\r\n", millis());
            stateMachine.setPhase(FlightPhase::ARMED);
        } else if (decoded.payload[0] == '0' || decoded.payload[0] == 0) {
            writeSystemLog("[%lu] CMD: DISARM command executed\r\n", millis());
            stateMachine.setPhase(FlightPhase::UNARMED);
        }
        
    } else if (idA == 'p' && idB == 'r') {
        // Ping request (pr) - status checks
        writeSystemLog("[%lu] CMD: Ping request (pr) received\r\n", millis());
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
        writeSystemLog("[%lu] WARN: Unknown command ID: %c%c\r\n", millis(), idA, idB);
    }
    
    // Log received packet info
    Serial.print("Sequence: ");
    Serial.print(decoded.sequenceID);
    Serial.print(", Timestamp: ");
    Serial.println(decoded.timestamp);
}
