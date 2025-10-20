#include <Arduino.h>
#include "RFD900x.h"

RFD900x::RFD900x(HardwareSerial& hwSerial) : serial(hwSerial) {}


void RFD900x::init(uint32_t baud) {
  serial.begin(baud);
  delay(100);
}

bool RFD900x::send(const uint8_t* buf, size_t len) {
    size_t wrote = serial.write(buf, len);
    serial.flush();
    return wrote == len;
}

size_t RFD900x::recv(uint8_t* buf, size_t len, uint32_t timeoutMs) {
    uint32_t t0 = millis();
    size_t idx = 0;
    while (idx < len && (millis() - t0) < timeoutMs) {
      if (serial.available()) {
        buf[idx++] = (uint8_t)serial.read();
      }
    }
    return idx;
  }


bool RFD900x::enterATMode(uint32_t silenceMs) {
    delay(silenceMs);
    serial.print("+++");
    delay(silenceMs);
    char resp[16] = {0};
    size_t r = recv((uint8_t*)resp, sizeof(resp)-1, 2000);
    String s = String(resp);
    return s.indexOf("OK") >= 0 || s.length() > 0;
}

