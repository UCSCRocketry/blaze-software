/**
 * @file FlightState.h
 * @brief Flight state machine for avionics system
 * 
 * Manages flight phases: UNARMED -> ARMED -> LAUNCH -> APOGEE -> DESCENT -> LANDED
 */

#pragma once

#include <Arduino.h>

/**
 * @enum FlightPhase
 * @brief Enumeration of all possible flight phases
 */
enum class FlightPhase : uint8_t {
    UNARMED = 0,    ///< System initialized but not ready for flight
    ARMED = 1,      ///< System ready, waiting for launch detection
    LAUNCH = 2,     ///< Launch detected, ascending
    APOGEE = 3,     ///< Apogee detected, preparing for descent
    DESCENT = 4,    ///< Descending under parachute
    LANDED = 5,     ///< Landed, mission complete
    ERROR = 6       ///< Error state
};

/**
 * @struct FlightState
 * @brief Current flight state information
 */
struct FlightState {
    FlightPhase phase;           ///< Current flight phase
    float altitude;              ///< Current altitude (m)
    float maxAltitude;           ///< Maximum altitude reached (m)
    uint32_t timestamp;          ///< Timestamp of last state update (ms)
    bool radioFlag;              ///< Radio transmission flag
    bool loggingEnabled;         ///< Logging enabled flag
    uint32_t launchTime;         ///< Time when launch was detected (ms)
    uint32_t apogeeTime;         ///< Time when apogee was detected (ms)
    uint32_t landedTime;         ///< Time when landing was detected (ms)
    bool errorFlag;              ///< Error flag
    char errorMessage[32];       ///< Error message if errorFlag is true
};

/**
 * @class FlightStateMachine
 * @brief State machine controller for flight phases
 */
class FlightStateMachine {
public:
    /**
     * @brief Constructor
     */
    FlightStateMachine();

    /**
     * @brief Initialize the state machine
     */
    void init();

    /**
     * @brief Update the state machine based on sensor data
     * @param altitude Current altitude (m)
     * @param acceleration Current acceleration magnitude (g)
     * @param velocity Current vertical velocity (m/s) - optional, can be calculated
     * @return true if state changed, false otherwise
     */
    bool update(float altitude, float acceleration, float velocity = 0.0f);

    /**
     * @brief Get current flight state
     * @return Reference to current FlightState
     */
    const FlightState& getState() const { return _state; }

    /**
     * @brief Get current flight phase
     * @return Current FlightPhase
     */
    FlightPhase getPhase() const { return _state.phase; }

    /**
     * @brief Manually set flight phase (for testing/override)
     * @param phase New flight phase
     */
    void setPhase(FlightPhase phase);

    /**
     * @brief Enable or disable logging
     * @param enable true to enable, false to disable
     */
    void setLoggingEnabled(bool enable);

    /**
     * @brief Enable or disable radio transmission
     * @param enable true to enable, false to disable
     */
    void setRadioFlag(bool enable);

    /**
     * @brief Reset state machine to UNARMED
     */
    void reset();

    /**
     * @brief Set error state with message
     * @param message Error message
     */
    void setError(const char* message);

private:
    FlightState _state;
    
    // State transition thresholds
    static constexpr float LAUNCH_ACCEL_THRESHOLD = 2.0f;  // g - acceleration threshold for launch detection
    static constexpr float APOGEE_VELOCITY_THRESHOLD = -0.5f;  // m/s - negative velocity threshold for apogee
    static constexpr float LANDED_ACCEL_THRESHOLD = 0.5f;  // g - low acceleration threshold for landing
    static constexpr float LANDED_ALTITUDE_THRESHOLD = 5.0f;  // m - altitude threshold for landing detection
    
    // Timing constants
    static constexpr uint32_t LAUNCH_DETECTION_TIME = 100;  // ms - time acceleration must be above threshold
    static constexpr uint32_t APOGEE_DETECTION_TIME = 500;   // ms - time velocity must be negative
    static constexpr uint32_t LANDED_DETECTION_TIME = 2000;  // ms - time conditions must be met for landing
    
    // State tracking
    uint32_t _launchDetectionStart;
    uint32_t _apogeeDetectionStart;
    uint32_t _landedDetectionStart;
    float _previousAltitude;
    float _previousVelocity;
    uint32_t _lastUpdateTime;
    
    /**
     * @brief Calculate vertical velocity from altitude change
     * @param currentAltitude Current altitude
     * @param deltaTime Time since last update (ms)
     * @return Vertical velocity (m/s)
     */
    float calculateVelocity(float currentAltitude, uint32_t deltaTime);
    
    /**
     * @brief Check if launch conditions are met
     * @param acceleration Current acceleration
     * @return true if launch detected
     */
    bool checkLaunchConditions(float acceleration);
    
    /**
     * @brief Check if apogee conditions are met
     * @param velocity Current vertical velocity
     * @return true if apogee detected
     */
    bool checkApogeeConditions(float velocity);
    
    /**
     * @brief Check if landing conditions are met
     * @param altitude Current altitude
     * @param acceleration Current acceleration
     * @return true if landing detected
     */
    bool checkLandedConditions(float altitude, float acceleration);
};
