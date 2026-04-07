#pragma once
#include <Arduino.h>
#include <RH_RF69.h>

class Radio {
public:
  explicit Radio(uint8_t CS, uint8_t INT, uint8_t RST);
  
  size_t max_message_length;
  String call_sign;

  void setCallSign(String sign);
  bool init(uint32_t freq);
  bool send(const uint8_t* buf, uint8_t len = 32, bool send_call_sign = false);
  size_t recv(uint8_t* buf, uint8_t len, uint32_t timeoutMs);
  size_t recv(uint8_t* buf, uint8_t len);
  bool available();

private:
  RH_RF69 radio;
  uint8_t cs_pin;
  uint8_t int_pin;
  uint8_t rst_pin; 
};
