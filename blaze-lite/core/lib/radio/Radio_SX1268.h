#pragma once
#include <Arduino.h>
#include <RadioLib.h>

/**
 * Radio driver for the Ebyte E22-400M33S (Semtech SX1268 + external PA/LNA, SPI).
 *
 * Drop-in replacement for the RFM69-based Radio class: same public API backed by RadioLib's SX1268 driver.
 * + some extra features
 */
class Radio_SX1268 {
public:
  /// LoRa modem settings. Both ends of the link must use identical values.
  struct LoRaConfig {
    float    bandwidthKHz   = 500.0f;  ///< 7.8 .. 500 kHz
    uint8_t  spreadingFactor = 7;      ///< 5 .. 12
    uint8_t  codingRate      = 5;      ///< 5..8 => 4/5 .. 4/8
    uint8_t  syncWord        = RADIOLIB_SX126X_SYNC_WORD_PRIVATE;
    uint16_t preambleLength  = 8;
  };

  /// SX1268 chip output power, NOT antenna power. The E22-400M33S PA adds ~25 dB,
  /// so 8 dBm here is ~33 dBm (2 W) at the antenna. Driving the PA harder risks damaging it.
  static constexpr int8_t MAX_CHIP_TX_POWER_DBM = 8;

  /// TCXO on the E22 is powered from SX1268 DIO3 at 1.8 V (per Ebyte manual).
  static constexpr float TCXO_VOLTAGE = 1.8f;

  explicit Radio_SX1268(uint8_t CS, uint8_t DIO1, uint8_t RST, uint8_t BUSY,
                        uint32_t TXEN, uint32_t RXEN, SPIClass& spi = SPI);

  size_t max_message_length;
  String call_sign;

  void setCallSign(String sign);
  bool init(float freq);  ///< Uses default LoRaConfig
  bool init(float freq, const LoRaConfig& config);
  bool setTxPower(int8_t chipPowerDbm);
  bool send(const uint8_t* buf, uint8_t len = 32, bool send_call_sign = false);
  size_t recv(uint8_t* buf, uint8_t len, uint32_t timeoutMs);
  size_t recv(uint8_t* buf, uint8_t len);
  bool available();

  /// RSSI (dBm) / SNR (dB) of the last received packet.
  float lastRssi();
  float lastSnr();

private:
  static void onDio1();
  static volatile bool packet_received;

  bool startReceive();

  Module module;
  SX1268 radio;
  uint32_t txen_pin;
  uint32_t rxen_pin;
};
