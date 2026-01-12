#pragma once
#include <Arduino.h>

class RFD900x {
public:
  explicit RFD900x(HardwareSerial& hwSerial);

  String continuing;

  static const size_t RX_BUFFER_SIZE = 256;
  static const size_t TX_BUFFER_SIZE = 512;
  static const size_t MAX_PACKET_SIZE = 32;

  void init(uint32_t baud = 57600);
  size_t recv(uint8_t* buf, size_t len, uint32_t timeoutMs);
  size_t recv(uint8_t* buf, size_t len);
  bool enterATMode(uint32_t silenceMs = 1200);
  void exitATMode();
  bool changeParameter(String parameter, String value, uint32_t silenceMs = 100);
  String getParameter(String parameter);
  bool changeNetID(int id);
  bool changeSerialSpeed(int speed);
  bool changeAirSpeed(int speed);
  String getNetID();
  String getSerialSpeed();
  String getAirSpeed();
  

  void handleATReceiving(char buf[]);
  
  // Buffer management
  size_t readFromRxBuffer(uint8_t* buf, size_t maxLen);
  size_t getFirstPacketLength();
  bool writeToTxBuffer(const uint8_t* buf, size_t len);
  void flushTxBuffer();
  size_t getTxBufferUsage();

private:
  HardwareSerial& serial;  // reference to hardware UART
  
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
};
