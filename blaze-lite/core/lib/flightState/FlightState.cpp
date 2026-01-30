/**
 * @file FlightState.cpp
 * @brief Implementation of FlightStateMachine
 */

#include "FlightState.h"

FlightStateMachine::FlightStateMachine() 
    : _state{}, _launchDetectionStart(0), _apogeeDetectionStart(0), 
      _landedDetectionStart(0), _previousAltitude(0.0f), _previousVelocity(0.0f),
      _lastUpdateTime(0)
{
    _state.phase = FlightPhase::UNARMED;
    _state.altitude = 0.0f;
    _state.maxAltitude = 0.0f;
    _state.timestamp = 0;
    _state.radioFlag = false;
    _state.loggingEnabled = false;
    _state.launchTime = 0;
    _state.apogeeTime = 0;
    _state.landedTime = 0;
    _state.errorFlag = false;
    memset(_state.errorMessage, 0, sizeof(_state.errorMessage));
}

void FlightStateMachine::init() {
    reset();
}

void FlightStateMachine::reset() {
    _state.phase = FlightPhase::UNARMED;
    _state.altitude = 0.0f;
    _state.maxAltitude = 0.0f;
    _state.timestamp = millis();
    _state.radioFlag = false;
    _state.loggingEnabled = false;
    _state.launchTime = 0;
    _state.apogeeTime = 0;
    _state.landedTime = 0;
    _state.errorFlag = false;
    memset(_state.errorMessage, 0, sizeof(_state.errorMessage));
    
    _launchDetectionStart = 0;
    _apogeeDetectionStart = 0;
    _landedDetectionStart = 0;
    _previousAltitude = 0.0f;
    _previousVelocity = 0.0f;
    _lastUpdateTime = millis();
}

bool FlightStateMachine::update(float altitude, float acceleration, float velocity) {
    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - _lastUpdateTime;
    _lastUpdateTime = currentTime;
    
    // Update state timestamp
    _state.timestamp = currentTime;
    _state.altitude = altitude;
    
    // Update max altitude
    if (altitude > _state.maxAltitude) {
        _state.maxAltitude = altitude;
    }
    
    // Calculate velocity if not provided
    if (velocity == 0.0f && deltaTime > 0) {
        velocity = calculateVelocity(altitude, deltaTime);
    }
    
    bool stateChanged = false;
    
    // State machine transitions
    switch (_state.phase) {
        case FlightPhase::UNARMED:
            
            
        case FlightPhase::ARMED:
            // Check for launch detection
            if (checkLaunchConditions(acceleration)) {
                _state.phase = FlightPhase::LAUNCH;
                _state.launchTime = currentTime;
                // Logging and radio already enabled when ARMED, keep them enabled
                stateChanged = true;
            }
            break;
            
        case FlightPhase::LAUNCH:
            // Check for apogee detection
            if (checkApogeeConditions(velocity)) {
                _state.phase = FlightPhase::APOGEE;
                _state.apogeeTime = currentTime;
                stateChanged = true;
            }
            break;
            
        case FlightPhase::APOGEE:
            // Transition to descent when velocity becomes positive (falling)
            if (velocity < -1.0f) {  // Falling down
                _state.phase = FlightPhase::DESCENT;
                stateChanged = true;
            }
            break;
            
        case FlightPhase::DESCENT:
            // Check for landing detection
            if (checkLandedConditions(altitude, acceleration)) {
                _state.phase = FlightPhase::LANDED;
                _state.landedTime = currentTime;
                stateChanged = true;
            }
            break;
            
        case FlightPhase::LANDED:
            // Stay in LANDED state
            break;
            
        case FlightPhase::ERROR:
            // Stay in ERROR state until reset
            break;
    }
    
    // Update previous values for next iteration
    _previousAltitude = altitude;
    _previousVelocity = velocity;
    
    return stateChanged;
}

void FlightStateMachine::setPhase(FlightPhase phase) {
    _state.phase = phase;
    _state.timestamp = millis();
    
    // Update logging and radio state based on phase
    if (phase == FlightPhase::ARMED) {
        _state.loggingEnabled = true;
        _state.radioFlag = true;
    } else if (phase == FlightPhase::UNARMED) {
        _state.loggingEnabled = false;
        _state.radioFlag = false;
    }
}

void FlightStateMachine::setLoggingEnabled(bool enable) {
    _state.loggingEnabled = enable;
}

void FlightStateMachine::setRadioFlag(bool enable) {
    _state.radioFlag = enable;
}

void FlightStateMachine::setError(const char* message) {
    _state.phase = FlightPhase::ERROR;
    _state.errorFlag = true;
    strncpy(_state.errorMessage, message, sizeof(_state.errorMessage) - 1);
    _state.errorMessage[sizeof(_state.errorMessage) - 1] = '\0';
    _state.timestamp = millis();
}

float FlightStateMachine::calculateVelocity(float currentAltitude, uint32_t deltaTime) {
    if (deltaTime == 0) {
        return _previousVelocity;
    }
    
    float altitudeChange = currentAltitude - _previousAltitude;
    float timeSeconds = deltaTime / 1000.0f;
    
    if (timeSeconds > 0) {
        return altitudeChange / timeSeconds;
    }
    
    return _previousVelocity;
}

bool FlightStateMachine::checkLaunchConditions(float acceleration) {
    if (acceleration > LAUNCH_ACCEL_THRESHOLD) {
        if (_launchDetectionStart == 0) {
            _launchDetectionStart = millis();
        } else if (millis() - _launchDetectionStart >= LAUNCH_DETECTION_TIME) {
            _launchDetectionStart = 0;
            return true;
        }
    } else {
        _launchDetectionStart = 0;
    }
    return false;
}

bool FlightStateMachine::checkApogeeConditions(float velocity) {
    if (velocity < APOGEE_VELOCITY_THRESHOLD) {
        if (_apogeeDetectionStart == 0) {
            _apogeeDetectionStart = millis();
        } else if (millis() - _apogeeDetectionStart >= APOGEE_DETECTION_TIME) {
            _apogeeDetectionStart = 0;
            return true;
        }
    } else {
        _apogeeDetectionStart = 0;
    }
    return false;
}

bool FlightStateMachine::checkLandedConditions(float altitude, float acceleration) {
    bool altitudeOk = altitude < LANDED_ALTITUDE_THRESHOLD;
    float accelDiff = (acceleration > 1.0f) ? (acceleration - 1.0f) : (1.0f - acceleration);
    bool accelOk = accelDiff < LANDED_ACCEL_THRESHOLD;  // ~1g when landed
    
    if (altitudeOk && accelOk) {
        if (_landedDetectionStart == 0) {
            _landedDetectionStart = millis();
        } else if (millis() - _landedDetectionStart >= LANDED_DETECTION_TIME) {
            _landedDetectionStart = 0;
            return true;
        }
    } else {
        _landedDetectionStart = 0;
    }
    return false;
}
