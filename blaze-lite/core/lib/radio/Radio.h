#pragma once
#include <Arduino.h>
#include <RH_RF69.h>

class Radio {
public:
  explicit Radio(uint8_t CS, uint8_t INT, uint8_t RST);

  /*
  
  static const size_t RX_BUFFER_SIZE = 256;
  static const size_t TX_BUFFER_SIZE = 512;
  static const size_t MAX_PACKET_SIZE = 32;

  */
  
  size_t max_message_length;
  String call_sign;

  void setCallSign(String sign);
  bool init(uint32_t freq);
  bool send(const uint8_t* buf, uint8_t len = 32, bool send_call_sign = false);
  size_t recv(uint8_t* buf, uint8_t len, uint32_t timeoutMs);
  size_t recv(uint8_t* buf, uint8_t len);
  bool available();

  /*
  
  // Buffer management
  size_t readFromRxBuffer(uint8_t* buf, size_t maxLen);
  size_t getFirstPacketLength();
  bool writeToTxBuffer(const uint8_t* buf, size_t len);
  void flushTxBuffer();
  size_t getTxBufferUsage();
  
  */

private:
  RH_RF69 radio;
  uint8_t cs_pin;
  uint8_t int_pin;
  uint8_t rst_pin; 

  /*
  
  // Circular RX buffer for incoming packets
  uint8_t rxBuffer[RX_BUFFER_SIZE];
  volatile size_t rxWriteIdx;
  size_t rxReadIdx;
  
  // TX buffer for outgoing data
  uint8_t txBuffer[TX_BUFFER_SIZE];
  volatile size_t txWriteIdx;
  size_t txReadIdx;
  
  // Static pointer for ISR
  static RFD900x* instance;
  
  // ISR handlers
  static void rxISR();
  void handleRxByte(uint8_t byte);
  
  */
};
