/**
 * @file LSM6DSOXIMU.h
 * @brief SPI wrapper for the Adafruit LSM6DSOX accelerometer and gyroscope.
 */

#pragma once

#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

// Override at build time if PB2 is occupied (e.g. by the KX134).
#ifndef LSM6DSOX_CS_PIN
#define LSM6DSOX_CS_PIN PB2
#endif

constexpr uint32_t LSM6DSOX_SPI_FREQUENCY = 1000000;
extern SPISettings lsm6dsoxSettings;

/**
 * @class LSM6DSOXIMU
 * @brief Guarded SPI interface using the project's zero-success status convention.
 */
class LSM6DSOXIMU {
public:
    /**
     * @brief Create an uninitialized IMU wrapper without accessing hardware.
     * No inputs. Call startUp() before accessing the sensor.
     * No return value.
     */
    LSM6DSOXIMU();

    /**
     * @brief Destroy the wrapper and its underlying driver.
     * No inputs. The caller owns the SPI port; it is not stopped.
     * No return value.
     */
    ~LSM6DSOXIMU();

    /**
     * @brief Initialize hardware SPI and verify the LSM6DSOX identity.
     * @param spiPort Configured SPI port that must outlive this object.
     * Uses LSM6DSOX_CS_PIN at 1 MHz, MSB first, mode 0. Restores driver
     * defaults: 104 Hz on both sensors, +/-4 g and +/-2000 degrees/s.
     * @return 0 on success; -1 if initialization fails.
     */
    int8_t startUp(SPIClass &spiPort);

    /**
     * @brief Return the cached initialization state without accessing SPI.
     * No inputs. This does not detect a disconnected device.
     * @return true after successful startup/reset; false otherwise.
     */
    bool isReady() const;

    /**
     * @brief Check initialization and read WHO_AM_I over SPI.
     * No inputs. This is an identity/communication check, not the sensor
     * excitation self-test; it does not validate measurement accuracy.
     * @return 0 if the device reports the LSM6DSOX ID; -1 otherwise.
     */
    int8_t selfTest();

    /**
     * @brief Reinitialize the device on the saved SPI port, including software reset.
     * No inputs. Requires successful startup. Restores startup defaults,
     * including data rates, ranges and block-data-update configuration.
     * Uses the underlying driver reset, which waits for hardware completion.
     * @return 0 on success; -1 if uninitialized or reinitialization fails.
     */
    int8_t reset();

    /**
     * @brief Read the WHO_AM_I register in a complete SPI transaction.
     * No inputs. SPI selects devices by chip-select, not an address; this
     * legacy method name returns the model ID, not a unique serial number.
     * @return 0x6C for an LSM6DSOX, another byte on mismatch, or 0 before startup.
     */
    uint8_t getAddress();

    /**
     * @brief Read the configured accelerometer output data rate through the driver.
     * No inputs. The result is an enum code, not a physical unit value.
     * @return lsm6ds_data_rate_t code, or -1 before initialization.
     */
    int8_t getAccelDataRate();

    /**
     * @brief Validate, write and read back the accelerometer output data rate.
     * @param value A lsm6ds_data_rate_t setting supported by the LSM6DSOX.
     * SHUTDOWN disables this sensor.
     * @return true if readback matches; false if uninitialized, invalid or mismatched.
     */
    bool setAccelDataRate(lsm6ds_data_rate_t value);

    /**
     * @brief Read the configured accelerometer full-scale range through the driver.
     * No inputs. The result is an enum code, not a physical unit value.
     * @return lsm6ds_accel_range_t code, or -1 before initialization.
     */
    int8_t getAccelRange();

    /**
     * @brief Validate, write and read back the accelerometer full-scale range.
     * @param value A lsm6ds_accel_range_t setting supported by the LSM6DSOX.
     * Supported ranges: +/-2, 4, 8 or 16 g.
     * @return true if readback matches; false if uninitialized, invalid or mismatched.
     */
    bool setAccelRange(lsm6ds_accel_range_t value);

    /**
     * @brief Read the configured gyroscope output data rate through the driver.
     * No inputs. The result is an enum code, not a physical unit value.
     * @return lsm6ds_data_rate_t code, or -1 before initialization.
     */
    int8_t getGyroDataRate();

    /**
     * @brief Validate, write and read back the gyroscope output data rate.
     * @param value A lsm6ds_data_rate_t setting supported by the LSM6DSOX.
     * SHUTDOWN disables this sensor.
     * @return true if readback matches; false if uninitialized, invalid or mismatched.
     */
    bool setGyroDataRate(lsm6ds_data_rate_t value);

    /**
     * @brief Read the configured gyroscope full-scale range through the driver.
     * No inputs. The result is an enum code, not a physical unit value.
     * @return lsm6ds_gyro_range_t code, or -1 before initialization.
     */
    int8_t getGyroRange();

    /**
     * @brief Validate, write and read back the gyroscope full-scale range.
     * @param value A lsm6ds_gyro_range_t setting supported by the LSM6DSOX.
     * Supported ranges: +/-125, 250, 500, 1000 or 2000 degrees/s.
     * The ISM330DHCX-only 4000 degrees/s setting is rejected.
     * @return true if readback matches; false if uninitialized, invalid or mismatched.
     */
    bool setGyroRange(lsm6ds_gyro_range_t value);

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
    bool getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                  sensors_event_t *temp);


    /**
     * @brief Read all three data-ready flags in one SPI transaction.
     * No inputs. Reading status does not read the measurement registers.
     * @return Bitmask: bit 0 acceleration, bit 1 gyro, bit 2 temperature;
     *         0 means no new data, -1 means uninitialized. SPI has no ACK,
     *         so a returned byte does not prove the device is connected.
     */
    int8_t getStatus();

    /**
     * @brief Check the accelerometer data-ready flag without consuming a sample.
     * No inputs; reads STATUS_REG through getStatus().
     * @return true if new data is available; false if not ready or uninitialized.
     */
    bool accelerationAvailable();

    /**
     * @brief Check the gyroscope data-ready flag without consuming a sample.
     * No inputs; reads STATUS_REG through getStatus().
     * @return true if new data is available; false if not ready or uninitialized.
     */
    bool gyroscopeAvailable();

    /**
     * @brief Convert the configured output-data-rate enum to Hz.
     * No inputs; reads the current setting through getAccelDataRate().
     * @return Rate in Hz, 0.0f for shutdown, or -1.0f if uninitialized or
     *         the register contains a code outside the supported enum.
     */
    float accelerationSampleRate();

    /**
     * @brief Convert the configured output-data-rate enum to Hz.
     * No inputs; reads the current setting through getGyroDataRate().
     * @return Rate in Hz, 0.0f for shutdown, or -1.0f if uninitialized or
     *         the register contains a code outside the supported enum.
     */
    float gyroscopeSampleRate();

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
    bool readAcceleration(float &x, float &y, float &z);

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
    bool readGyroscope(float &x, float &y, float &z);

    /**
     * @brief Set INT1/INT2 polarity and output type while preserving other CTRL3_C bits.
     * @param activeLow true for active-low, false for active-high outputs.
     * @param openDrain true for open-drain (requires a pull-up), false for push-pull.
     * Configures sensor pins only; does not install a microcontroller ISR.
     * @return true if the changed bits read back correctly; false if uninitialized
     *         or readback differs. No bus-level acknowledgement is available.
     */
    bool configIntOutputs(bool activeLow, bool openDrain);

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
    bool configInt1(bool gyroReady, bool accelReady, bool wakeup = false);

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
    bool configInt2(bool tempReady, bool gyroReady, bool accelReady);

private:
    /**
     * @brief Read one register using the saved SPI port and a complete CS transaction.
     * @param address Register address in the active bank; caller must ensure startup
     *        succeeded. This helper does not select a register bank.
     * @return Received byte; SPI provides no device acknowledgement.
     */
    uint8_t readRegister(uint8_t address);

    /**
     * @brief Read-modify-write configuration bits, then verify those bits by readback.
     * @param address Main-bank register address; use only ordinary configuration
     *        registers, not read-to-clear or self-clearing command bits.
     * @param mask Bits to change; all other bits retain their current values.
     * @param value Requested bits; bits outside mask are ignored.
     * @return false before initialization or on readback mismatch; true otherwise.
     *         Caller must serialize access with other users of this sensor.
     */
    bool updateRegister(uint8_t address, uint8_t mask, uint8_t value);

    Adafruit_LSM6DSOX _lsm6dsox; ///< Underlying Adafruit sensor driver.
    SPIClass *_spiPort;          ///< Caller-owned SPI port retained for reset/ID.
    bool _initialized;           ///< True only after successful initialization.
};
