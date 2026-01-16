#include <Arduino.h>
#include "Radio.h"
#include <RH_RF69.h>
#include <SPI.h>

Radio::Radio(uint8_t CS, uint8_t INT, uint8_t RST) : cs_pin(CS), int_pin(INT), rst_pin(RST), radio(CS, INT) {
  max_message_length = RH_RF69_MAX_MESSAGE_LEN;
  call_sign = "";
}

/**
 * For long range transmission, a call sign is required.
 */
void Radio::setCallSign(String sign) {
  call_sign = sign + ":";
}

/**
 * Initializes radio. Handles reset pin, frequency, and power config.
 */
bool Radio::init(uint32_t freq) {
  SPI.begin();

  pinMode(rst_pin, OUTPUT);
  digitalWrite(rst_pin, LOW);

  // manual reset
  digitalWrite(rst_pin, HIGH);
  delay(10);
  digitalWrite(rst_pin, LOW);
  delay(10);
  
  bool initialized = radio.init();

  if(!initialized) {
    Serial.println("Failed to init radio!");
  }

  bool freqSet = radio.setFrequency(freq);

  if(!freqSet) {
    Serial.println("Failed to set frequency!");
  }

  radio.setTxPower(20, true);
  
  return true;
}

/**
 * Send a packet and wait for radio to send.
 */
bool Radio::send(const uint8_t* buf, uint8_t len, bool send_call_sign) {
  if (!call_sign.equals("") && send_call_sign) {
    uint8_t tempBuf[len + call_sign.length() + 1];
    memcpy(tempBuf, call_sign.c_str(), call_sign.length());
    memcpy(tempBuf + call_sign.length(), buf, len);
    return radio.send(tempBuf, len + call_sign.length()) && radio.waitPacketSent();
  }

  return radio.send(buf, len) && radio.waitPacketSent();
}

/**
 * Wait for message to be received.
 */
size_t Radio::recv(uint8_t* buf, uint8_t len, uint32_t timeoutMs) {
    uint32_t t0 = millis();
    while ((millis() - t0) < timeoutMs) {
      if (radio.available()) {
        if(radio.recv(buf, &len)) {
          return len;
        };
      }
    }
    return 0;
  }

/**
 * Check for received packets.
 */
size_t Radio::recv(uint8_t* buf, uint8_t len) {
  if(radio.recv(buf, &len)) {
    return len;
  }
  return 0;
}

/**
 * Checks if a message has been received.
 */
bool Radio::available() {
  return radio.available();
}