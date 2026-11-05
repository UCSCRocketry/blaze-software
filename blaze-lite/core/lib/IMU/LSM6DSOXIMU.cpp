/**
 * @file LSM6DSOXIMU.cpp
 * @brief Implementation of the LSM6DSOX SPI wrapper.
 */

#include "LSM6DSOXIMU.h"

// Match the settings used internally by Adafruit_LSM6DS::begin_SPI().
SPISettings lsm6dsoxSettings(LSM6DSOX_SPI_FREQUENCY, MSBFIRST, SPI_MODE0);

/**
 * @brief Create an uninitialized IMU wrapper without accessing hardware.
 * No inputs. Call startUp() before accessing the sensor.
 * No return value.
 */
LSM6DSOXIMU::LSM6DSOXIMU() : _spiPort(nullptr), _initialized(false) {
}

/**
 * @brief Destroy the wrapper and its underlying driver.
 * No inputs. The caller owns the SPI port; it is not stopped.
 * No return value.
 */
LSM6DSOXIMU::~LSM6DSOXIMU() {
}

/**
 * @brief Initialize hardware SPI and verify the LSM6DSOX identity.
 * @param spiPort Configured SPI port that must outlive this object.
 * Uses LSM6DSOX_CS_PIN at 1 MHz, MSB first, mode 0. Restores driver
 * defaults: 104 Hz on both sensors, +/-4 g and +/-2000 degrees/s.
 * @return 0 on success; -1 if initialization fails.
 */
int8_t LSM6DSOXIMU::startUp(SPIClass &spiPort) {
    _initialized = false;
    _spiPort = &spiPort;
    _initialized = _lsm6dsox.begin_SPI(LSM6DSOX_CS_PIN, _spiPort, 0,
                                    LSM6DSOX_SPI_FREQUENCY);
    return _initialized ? 0 : -1;
}

/**
 * @brief Return the cached initialization state without accessing SPI.
 * No inputs. This does not detect a disconnected device.
 * @return true after successful startup/reset; false otherwise.
 */
bool LSM6DSOXIMU::isReady() const {
    return _initialized;
}

/**
 * @brief Check initialization and read WHO_AM_I over SPI.
 * No inputs. This is an identity/communication check, not the sensor
 * excitation self-test; it does not validate measurement accuracy.
 * @return 0 if the device reports the LSM6DSOX ID; -1 otherwise.
 */
int8_t LSM6DSOXIMU::selfTest() {
    return (_initialized && getAddress() == LSM6DSOX_CHIP_ID) ? 0 : -1;
}

/**
 * @brief Reinitialize the device on the saved SPI port, including software reset.
 * No inputs. Requires successful startup. Restores startup defaults,
 * including data rates, ranges and block-data-update configuration.
 * Uses the underlying driver reset, which waits for hardware completion.
 * @return 0 on success; -1 if uninitialized or reinitialization fails.
 */
int8_t LSM6DSOXIMU::reset() {
    if (!_initialized || _spiPort == nullptr) {
        return -1;
    }
    return startUp(*_spiPort);
}

/**
 * @brief Read the WHO_AM_I register in a complete SPI transaction.
 * No inputs. SPI selects devices by chip-select, not an address; this
 * legacy method name returns the model ID, not a unique serial number.
 * @return 0x6C for an LSM6DSOX, another byte on mismatch, or 0 before startup.
 */
uint8_t LSM6DSOXIMU::getAddress() {
    if (!_initialized) {
        return 0;
    }
    return readRegister(LSM6DS_WHOAMI);
}

/**
 * @brief Read the configured accelerometer output data rate through the driver.
 * No inputs. The result is an enum code, not a physical unit value.
 * @return lsm6ds_data_rate_t code, or -1 before initialization.
 */
int8_t LSM6DSOXIMU::getAccelDataRate() {
    if (!_initialized) {
        return -1;
    }
    return static_cast<int8_t>(_lsm6dsox.getAccelDataRate());
}

/**
 * @brief Validate, write and read back the accelerometer output data rate.
 * @param value A lsm6ds_data_rate_t setting supported by the LSM6DSOX.
 * SHUTDOWN disables this sensor.
 * @return true if readback matches; false if uninitialized, invalid or mismatched.
 */
bool LSM6DSOXIMU::setAccelDataRate(lsm6ds_data_rate_t value) {
    if (!_initialized || !(value >= LSM6DS_RATE_SHUTDOWN && value <= LSM6DS_RATE_6_66K_HZ)) {
        return false;
    }
    _lsm6dsox.setAccelDataRate(value);
    return _lsm6dsox.getAccelDataRate() == value;
}

/**
 * @brief Read the configured accelerometer full-scale range through the driver.
 * No inputs. The result is an enum code, not a physical unit value.
 * @return lsm6ds_accel_range_t code, or -1 before initialization.
 */
int8_t LSM6DSOXIMU::getAccelRange() {
    if (!_initialized) {
        return -1;
    }
    return static_cast<int8_t>(_lsm6dsox.getAccelRange());
}

/**
 * @brief Validate, write and read back the accelerometer full-scale range.
 * @param value A lsm6ds_accel_range_t setting supported by the LSM6DSOX.
 * Supported ranges: +/-2, 4, 8 or 16 g.
 * @return true if readback matches; false if uninitialized, invalid or mismatched.
 */
bool LSM6DSOXIMU::setAccelRange(lsm6ds_accel_range_t value) {
    if (!_initialized || !(value >= LSM6DS_ACCEL_RANGE_2_G && value <= LSM6DS_ACCEL_RANGE_8_G)) {
        return false;
    }
    _lsm6dsox.setAccelRange(value);
    return _lsm6dsox.getAccelRange() == value;
}

/**
 * @brief Read the configured gyroscope output data rate through the driver.
 * No inputs. The result is an enum code, not a physical unit value.
 * @return lsm6ds_data_rate_t code, or -1 before initialization.
 */
int8_t LSM6DSOXIMU::getGyroDataRate() {
    if (!_initialized) {
        return -1;
    }
    return static_cast<int8_t>(_lsm6dsox.getGyroDataRate());
}

/**
 * @brief Validate, write and read back the gyroscope output data rate.
 * @param value A lsm6ds_data_rate_t setting supported by the LSM6DSOX.
 * SHUTDOWN disables this sensor.
 * @return true if readback matches; false if uninitialized, invalid or mismatched.
 */
bool LSM6DSOXIMU::setGyroDataRate(lsm6ds_data_rate_t value) {
    if (!_initialized || !(value >= LSM6DS_RATE_SHUTDOWN && value <= LSM6DS_RATE_6_66K_HZ)) {
        return false;
    }
    _lsm6dsox.setGyroDataRate(value);
    return _lsm6dsox.getGyroDataRate() == value;
}

/**
 * @brief Read the configured gyroscope full-scale range through the driver.
 * No inputs. The result is an enum code, not a physical unit value.
 * @return lsm6ds_gyro_range_t code, or -1 before initialization.
 */
int8_t LSM6DSOXIMU::getGyroRange() {
    if (!_initialized) {
        return -1;
    }
    return static_cast<int8_t>(_lsm6dsox.getGyroRange());
}

/**
 * @brief Validate, write and read back the gyroscope full-scale range.
 * @param value A lsm6ds_gyro_range_t setting supported by the LSM6DSOX.
 * Supported ranges: +/-125, 250, 500, 1000 or 2000 degrees/s.
 * The ISM330DHCX-only 4000 degrees/s setting is rejected.
 * @return true if readback matches; false if uninitialized, invalid or mismatched.
 */
bool LSM6DSOXIMU::setGyroRange(lsm6ds_gyro_range_t value) {
    if (!_initialized || !(value == LSM6DS_GYRO_RANGE_125_DPS ||
          value == LSM6DS_GYRO_RANGE_250_DPS ||
          value == LSM6DS_GYRO_RANGE_500_DPS ||
          value == LSM6DS_GYRO_RANGE_1000_DPS ||
          value == LSM6DS_GYRO_RANGE_2000_DPS)) {
        return false;
    }
    _lsm6dsox.setGyroRange(value);
    return _lsm6dsox.getGyroRange() == value;
}

/**
 * @brief Read acceleration, angular velocity and temperature in one driver call.
 * @param accel Non-null output event; acceleration is in m/s^2.
 * @param gyro Non-null output event; angular velocity is in rad/s.
 * @param temp Non-null output event; temperature is in degrees Celsius.
 * Supply three distinct events. Outputs are unchanged on rejected inputs.
 * Reads the latest sample without waiting for new data; the Adafruit
 * driver does not report SPI transfer failures through getEvent().
 * @return false if uninitialized or pointers are null/aliased; otherwise
 * the driver result (not a guarantee of a fresh sample or bus health).
 */
bool LSM6DSOXIMU::getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                          sensors_event_t *temp) {
    if (!_initialized || accel == nullptr || gyro == nullptr || temp == nullptr ||
        accel == gyro || accel == temp || gyro == temp) {
        return false;
    }
    return _lsm6dsox.getEvent(accel, gyro, temp);
}

/**
 * @brief Read all three data-ready flags in one SPI transaction.
 * No inputs. Reading status does not read the measurement registers.
 * @return Bitmask: bit 0 acceleration, bit 1 gyro, bit 2 temperature;
 *         0 means no new data, -1 means uninitialized. SPI has no ACK,
 *         so a returned byte does not prove the device is connected.
 */
int8_t LSM6DSOXIMU::getStatus() {
    if (!_initialized) {
        return -1;
    }
    return static_cast<int8_t>(readRegister(LSM6DS_STATUS_REG) & 0x07);
}

/**
 * @brief Check the accelerometer data-ready flag without consuming a sample.
 * No inputs; reads STATUS_REG through getStatus().
 * @return true if new data is available; false if not ready or uninitialized.
 */
bool LSM6DSOXIMU::accelerationAvailable() {
    int8_t status = getStatus();
    return status >= 0 && (status & 0x01) != 0;
}

/**
 * @brief Check the gyroscope data-ready flag without consuming a sample.
 * No inputs; reads STATUS_REG through getStatus().
 * @return true if new data is available; false if not ready or uninitialized.
 */
bool LSM6DSOXIMU::gyroscopeAvailable() {
    int8_t status = getStatus();
    return status >= 0 && (status & 0x02) != 0;
}

/**
 * @brief Convert the configured output-data-rate enum to Hz.
 * No inputs; reads the current setting through getAccelDataRate().
 * @return Rate in Hz, 0.0f for shutdown, or -1.0f if uninitialized or
 *         the register contains a code outside the supported enum.
 */
float LSM6DSOXIMU::accelerationSampleRate() {
    static const float rates[] = {
        0.0f, 12.5f, 26.0f, 52.0f, 104.0f, 208.0f,
        416.0f, 833.0f, 1660.0f, 3330.0f, 6660.0f
    };
    int8_t rate = getAccelDataRate();
    if (rate < 0 || rate > LSM6DS_RATE_6_66K_HZ) {
        return -1.0f;
    }
    return rates[rate];
}

/**
 * @brief Convert the configured output-data-rate enum to Hz.
 * No inputs; reads the current setting through getGyroDataRate().
 * @return Rate in Hz, 0.0f for shutdown, or -1.0f if uninitialized or
 *         the register contains a code outside the supported enum.
 */
float LSM6DSOXIMU::gyroscopeSampleRate() {
    static const float rates[] = {
        0.0f, 12.5f, 26.0f, 52.0f, 104.0f, 208.0f,
        416.0f, 833.0f, 1660.0f, 3330.0f, 6660.0f
    };
    int8_t rate = getGyroDataRate();
    if (rate < 0 || rate > LSM6DS_RATE_6_66K_HZ) {
        return -1.0f;
    }
    return rates[rate];
}

/**
 * @brief Read the latest sample using Adafruit's range-aware event conversion.
 * @param x Output X axis in m/s^2.
 * @param y Output Y axis in m/s^2.
 * @param z Output Z axis in m/s^2; all three outputs must be distinct.
 * Uses getEvent(), which reads ALL sensors and can consume their data-ready
 * flags. Use getEvent() directly when both acceleration and gyro are needed
 * from the same read. Does not wait for a new sample.
 * @return false on uninitialized/aliased outputs (left unchanged), otherwise
 *         the event-read result; Adafruit does not report SPI read failures.
 */
bool LSM6DSOXIMU::readAcceleration(float &x, float &y, float &z) {
    if (!_initialized || &x == &y || &x == &z || &y == &z) {
        return false;
    }
    sensors_event_t accel, gyro, temp;
    if (!getEvent(&accel, &gyro, &temp)) {
        return false;
    }
    x = accel.acceleration.x;
    y = accel.acceleration.y;
    z = accel.acceleration.z;
    return true;
}

/**
 * @brief Read the latest sample using Adafruit's range-aware event conversion.
 * @param x Output X axis in rad/s.
 * @param y Output Y axis in rad/s.
 * @param z Output Z axis in rad/s; all three outputs must be distinct.
 * Uses getEvent(), which reads ALL sensors and can consume their data-ready
 * flags. Use getEvent() directly when both acceleration and gyro are needed
 * from the same read. Does not wait for a new sample.
 * @return false on uninitialized/aliased outputs (left unchanged), otherwise
 *         the event-read result; Adafruit does not report SPI read failures.
 */
bool LSM6DSOXIMU::readGyroscope(float &x, float &y, float &z) {
    if (!_initialized || &x == &y || &x == &z || &y == &z) {
        return false;
    }
    sensors_event_t accel, gyro, temp;
    if (!getEvent(&accel, &gyro, &temp)) {
        return false;
    }
    x = gyro.gyro.x;
    y = gyro.gyro.y;
    z = gyro.gyro.z;
    return true;
}

/**
 * @brief Set INT1/INT2 polarity and output type while preserving other CTRL3_C bits.
 * @param activeLow true for active-low, false for active-high outputs.
 * @param openDrain true for open-drain (requires a pull-up), false for push-pull.
 * Configures sensor pins only; does not install a microcontroller ISR.
 * @return true if the changed bits read back correctly; false if uninitialized
 *         or readback differs. No bus-level acknowledgement is available.
 */
bool LSM6DSOXIMU::configIntOutputs(bool activeLow, bool openDrain) {
    return updateRegister(LSM6DS_CTRL3_C, 0x30,
                          (activeLow ? 0x20 : 0) | (openDrain ? 0x10 : 0));
}

/**
 * @brief Route gyro/accelerometer data-ready and wake-up events to INT1.
 * @param gyroReady Enable gyro data-ready routing.
 * @param accelReady Enable accelerometer data-ready routing.
 * @param wakeup Enable wake-up routing; the motion detector must be configured
 *        separately. This does not enable detection or configure an MCU ISR.
 * Updates INT1_CTRL bits 1:0 and MD1_CFG bit 5, preserving other routes.
 * Unlike the shared Adafruit API, no temperature or step argument is offered:
 * INT1_CTRL bits 2 and 7 have different meanings on the LSM6DSOX.
 * @return true if both updates verify; false if uninitialized or mismatched.
 *         A failure can leave the first register updated; there is no rollback.
 */
bool LSM6DSOXIMU::configInt1(bool gyroReady, bool accelReady, bool wakeup) {
    if (!updateRegister(LSM6DS_INT1_CTRL, 0x03,
                        (gyroReady ? 0x02 : 0) | (accelReady ? 0x01 : 0))) {
        return false;
    }
    return updateRegister(LSM6DS_MD1_CFG, 0x20, wakeup ? 0x20 : 0);
}

/**
 * @brief Route temperature/gyro/accelerometer data-ready events to INT2.
 * @param tempReady Enable temperature data-ready routing.
 * @param gyroReady Enable gyro data-ready routing.
 * @param accelReady Enable accelerometer data-ready routing.
 * Updates INT2_CTRL bits 2:0, preserving unrelated routes. Assumes the default
 * INT2_ON_INT1=0; does not configure a microcontroller ISR.
 * @return true if changed bits read back correctly; false if uninitialized
 *         or readback differs.
 */
bool LSM6DSOXIMU::configInt2(bool tempReady, bool gyroReady, bool accelReady) {
    return updateRegister(LSM6DS_INT2_CTRL, 0x07,
                          (tempReady ? 0x04 : 0) | (gyroReady ? 0x02 : 0) |
                          (accelReady ? 0x01 : 0));
}

/**
 * @brief Read one register using the saved SPI port and a complete CS transaction.
 * @param address Register address in the active bank; caller must ensure startup
 *        succeeded. This helper does not select a register bank.
 * @return Received byte; SPI provides no device acknowledgement.
 */
uint8_t LSM6DSOXIMU::readRegister(uint8_t address) {
    _spiPort->beginTransaction(lsm6dsoxSettings);
    digitalWrite(LSM6DSOX_CS_PIN, LOW);
    _spiPort->transfer(address | 0x80);
    uint8_t value = _spiPort->transfer(0x00);
    digitalWrite(LSM6DSOX_CS_PIN, HIGH);
    _spiPort->endTransaction();
    return value;
}

/**
 * @brief Read-modify-write configuration bits, then verify those bits by readback.
 * @param address Main-bank register address; use only ordinary configuration
 *        registers, not read-to-clear or self-clearing command bits.
 * @param mask Bits to change; all other bits retain their current values.
 * @param value Requested bits; bits outside mask are ignored.
 * @return false before initialization or on readback mismatch; true otherwise.
 *         Caller must serialize access with other users of this sensor.
 */
bool LSM6DSOXIMU::updateRegister(uint8_t address, uint8_t mask, uint8_t value) {
    if (!_initialized) {
        return false;
    }
    uint8_t updated = (readRegister(address) & ~mask) | (value & mask);
    _spiPort->beginTransaction(lsm6dsoxSettings);
    digitalWrite(LSM6DSOX_CS_PIN, LOW);
    _spiPort->transfer(address & 0x7F);
    _spiPort->transfer(updated);
    digitalWrite(LSM6DSOX_CS_PIN, HIGH);
    _spiPort->endTransaction();
    return (readRegister(address) & mask) == (value & mask);
}
