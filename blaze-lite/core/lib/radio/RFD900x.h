#pragma once
#include <Arduino.h>

class RFD900x {
public:
  explicit RFD900x(HardwareSerial& hwSerial);
  void init(uint32_t baud = 57600);
  bool send(const uint8_t* buf, size_t len = 32);
  size_t recv(uint8_t* buf, size_t len, uint32_t timeoutMs = 3000);
  bool enterATMode(uint32_t silenceMs = 1200);

private:
  HardwareSerial& serial;  // reference to hardware UART
};
