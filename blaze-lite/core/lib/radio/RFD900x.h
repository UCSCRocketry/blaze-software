#pragma once
#include <Arduino.h>

class RFD900x {
public:
  explicit RFD900x(HardwareSerial& hwSerial);

  String continuing;

  void init(uint32_t baud = 57600);
  bool send(const uint8_t* buf, size_t len = 32);
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

  void handleReceiving(char buf[]);
  

private:
  HardwareSerial& serial;  // reference to hardware UART
};
