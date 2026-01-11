#include <Arduino.h>
#include "RFD900x.h"

RFD900x::RFD900x(HardwareSerial& hwSerial) : serial(hwSerial) {
    String continuing = "";
}

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

size_t RFD900x::recv(uint8_t* buf, size_t len) {
    size_t idx = 0;
    while (idx < len && serial.available()) {
      buf[idx++] = (uint8_t)serial.read();
    }
    return idx;
  }


bool RFD900x::enterATMode(uint32_t silenceMs) {
    delay(silenceMs);
    serial.print("+++");
    delay(silenceMs);
    char resp[16] = {0};
    size_t r = recv((uint8_t*)resp, sizeof(resp)-1, 1000);
    String s = String(resp);
    return s.indexOf("OK") >= 0 || s.length() > 0;
}

void RFD900x::exitATMode() {
  serial.println("AT&W\r");
  serial.println("ATZ\r");
}

String RFD900x::getParameter(String parameter) {
    serial.println("ATS" + parameter + "?\r");

    char resp[16] = {0};
    size_t r = recv((uint8_t*)resp, sizeof(resp)-1, 250);
    String s = String(resp);

    return s.substring(s.indexOf("\n") + 1, s.lastIndexOf("\n"));
}

bool RFD900x::changeParameter(String parameter, String value, uint32_t silenceMs) {
    serial.println("ATS" + parameter + "=" + value + "\r");

    delay(silenceMs);

    serial.println("ATS" + parameter + "?\r");

    char resp[16] = {0};
    size_t r = recv((uint8_t*)resp, sizeof(resp)-1, 250);
    String s = String(resp);

    return s.indexOf(value) >= 0;
}

bool RFD900x::changeNetID(int id) {
    return changeParameter("3", String(id));
}

bool RFD900x::changeSerialSpeed(int speed) {
    return changeParameter("1", String(speed));
}

bool RFD900x::changeAirSpeed(int speed) {
    return changeParameter("2", String(speed));
}

String RFD900x::getNetID() {
    return getParameter("3");
}

String RFD900x::getSerialSpeed() {
    return getParameter("1");
}

String RFD900x::getAirSpeed() {
    return getParameter("2");
}

void RFD900x::handleReceiving(char buf[]) {
    String result = "\n";
    const char* start = "rocket";
    const char* end = "end";

    continuing += String(buf);

    int startIndex = continuing.indexOf(start);
    int endIndex = -1;
    if (startIndex != -1) {
        endIndex = continuing.indexOf(end, startIndex);
    }

    if (startIndex != -1 && endIndex != -1 && enterATMode(1200)) {
        String command = continuing.substring(startIndex + strlen(start), endIndex);
        continuing = continuing.substring(endIndex + strlen(end));

        if (command.startsWith("-") && command.endsWith("-")) {
            command = command.substring(1, command.length() - 1);

            int lastIndex = 0;
            while (lastIndex < command.length()) {
                int ampersandIndex = command.indexOf('&', lastIndex);
                if (ampersandIndex == -1) {
                    ampersandIndex = command.length();
                }

                String pair = command.substring(lastIndex, ampersandIndex);
                int equalsIndex = pair.indexOf('=');
                int questionIndex = pair.indexOf('?');

                if (equalsIndex != -1) {
                    String key = pair.substring(0, equalsIndex);
                    String value = pair.substring(equalsIndex + 1);

                    Serial.print("Received command: ");
                    Serial.print(key);
                    Serial.print(" = ");
                    Serial.println(value);

                    if (key == "netid") {
                        int netId = value.toInt();
                        changeNetID(netId);
                    } else if (key == "serialspeed") {
                        int speed = value.toInt();
                        changeSerialSpeed(speed);
                    } else if (key == "airspeed") {
                        int speed = value.toInt();
                        changeAirSpeed(speed);
                    }
                } else if (questionIndex != -1) {
                    String key = pair.substring(0, questionIndex);

                    Serial.print("Received query for: ");
                    Serial.println(key);

                    if (key == "netid") {
                        String currentValue = String(getNetID());
                        result += "netid=" + currentValue + "\n";
                    } else if(key == "serialspeed") {
                        String currentValue = String(getSerialSpeed());
                        result += "serialspeed=" + currentValue + "\n";
                    } else if(key == "airspeed") {
                        String currentValue = String(getAirSpeed());
                        result += "airspeed=" + currentValue + "\n";
                    }
                }
                lastIndex = ampersandIndex + 1;
            }
        }

        Serial.println("Result of query: " + result);

        exitATMode();

        delay(5000);

        Serial.println(send((const uint8_t*)result.c_str(), result.length()));

        continuing = "";
    }
}