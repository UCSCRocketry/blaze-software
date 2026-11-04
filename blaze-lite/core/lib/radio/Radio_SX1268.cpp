#include <Arduino.h>
#include "Radio_SX1268.h"
#include <SPI.h>

volatile bool Radio_SX1268::packet_received = false;

/**
 * DIO1 interrupt: fires on RX done (and TX done during send()).
 * Only sets a flag - all SPI traffic happens in the main loop, so the radio
 * never touches the shared SPI bus from an ISR (SD card, flash, baro are on it too).
 */
void Radio_SX1268::onDio1() {
  packet_received = true;
}

Radio_SX1268::Radio_SX1268(uint8_t CS, uint8_t DIO1, uint8_t RST, uint8_t BUSY,
                           uint32_t TXEN, uint32_t RXEN, SPIClass& spi)
    : module(CS, DIO1, RST, BUSY, spi), radio(&module), txen_pin(TXEN), rxen_pin(RXEN) {
  max_message_length = RADIOLIB_SX126X_MAX_PACKET_LENGTH;
  call_sign = "";
}

/**
 * For long range transmission, a call sign is required.
 */
void Radio_SX1268::setCallSign(String sign) {
  call_sign = sign;
}

bool Radio_SX1268::init(float freq) {
  return init(freq, LoRaConfig());
}

/**
 * Initializes radio. Handles reset, TCXO, RF switch, frequency, modem and power config,
 * then leaves the radio in continuous receive.
 */
bool Radio_SX1268::init(float freq, const LoRaConfig& config) {
  // E22-400M33S is band-limited to 410-493 MHz by its filter/PA, even though SX1268 goes to 810 MHz.
  if (freq < 410.0f || freq > 493.0f) {
    Serial.println("Radio frequency outside E22-400M33S range (410-493 MHz)!");
    return false;
  }

  // TXEN/RXEN are toggled automatically by RadioLib on every mode change.
  radio.setRfSwitchPins(rxen_pin, txen_pin);

  // begin() resets the chip via NRST, configures TCXO on DIO3, runs calibration
  // (including image calibration for freq) and sets the modem parameters.
  int16_t state = radio.begin(freq,
                              config.bandwidthKHz,
                              config.spreadingFactor,
                              config.codingRate,
                              config.syncWord,
                              MAX_CHIP_TX_POWER_DBM,
                              config.preambleLength,
                              TCXO_VOLTAGE);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Failed to init radio! RadioLib error ");
    Serial.println(state);

    return false;
  }

  radio.setPacketReceivedAction(onDio1);

  return startReceive();
}

/**
 * Sets SX1268 chip output power in dBm (-9 .. MAX_CHIP_TX_POWER_DBM).
 * Clamped to protect the E22's external PA.
 */
bool Radio_SX1268::setTxPower(int8_t chipPowerDbm) {
  if (chipPowerDbm > MAX_CHIP_TX_POWER_DBM) {
    chipPowerDbm = MAX_CHIP_TX_POWER_DBM;
  }

  return radio.setOutputPower(chipPowerDbm) == RADIOLIB_ERR_NONE;
}

/**
 * Send a packet and wait for radio to send. Returns to receive afterwards.
 */
bool Radio_SX1268::send(const uint8_t* buf, uint8_t len, bool send_call_sign) {
  int16_t state;

  if (!call_sign.equals("") && send_call_sign) {
    size_t total = len + call_sign.length();
    if (total > max_message_length) {
      return false;
    }

    uint8_t tempBuf[total];
    memcpy(tempBuf, call_sign.c_str(), call_sign.length());
    memcpy(tempBuf + call_sign.length(), buf, len);
    state = radio.transmit(tempBuf, total);
  } else {
    state = radio.transmit(buf, len);
  }

  // TX-done also raises DIO1 and our ISR; it is not a received packet.
  packet_received = false;
  startReceive();

  return state == RADIOLIB_ERR_NONE;
}

/**
 * Wait for message to be received.
 */
size_t Radio_SX1268::recv(uint8_t* buf, uint8_t len, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  while ((millis() - t0) < timeoutMs) {
    if (available()) {
      size_t received = recv(buf, len);
      if (received > 0) {
        return received;
      }
    }
  }
  return 0;
}

/**
 * Check for received packets. Returns 0 if nothing was received or the CRC failed.
 */
size_t Radio_SX1268::recv(uint8_t* buf, uint8_t len) {
  // len == 0 would make readData() copy the whole packet into buf.
  if (!packet_received || len == 0) {
    return 0;
  }
  packet_received = false;

  size_t packetLen = radio.getPacketLength();
  int16_t state = radio.readData(buf, len);

  // Continuous RX keeps the radio listening after readData(); no restart needed.
  if (state != RADIOLIB_ERR_NONE) {
    return 0;
  }

  return packetLen < len ? packetLen : len;
}


// Checks if a message has been received.
bool Radio_SX1268::available() {
  return packet_received;
}

/// RSSI (dBm)of the last received packet.
float Radio_SX1268::lastRssi() {
  return radio.getRSSI();
}

// SNR (dB) of the last received packet.
float Radio_SX1268::lastSnr() {
  return radio.getSNR();
}

/// Start continuous receive mode. Returns true if successful.
bool Radio_SX1268::startReceive() {
  int16_t state = radio.startReceive();

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Failed to start receive! RadioLib error ");
    Serial.println(state);

    return false;
  }

  return true;
}
