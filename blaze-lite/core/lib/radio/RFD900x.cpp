#include <Arduino.h>
#include "RFD900x.h"

// Static member initialization
RFD900x* RFD900x::instance = nullptr;

RFD900x::RFD900x(HardwareSerial& hwSerial) : serial(hwSerial),
    rxWriteIdx(0), rxReadIdx(0), txWriteIdx(0), txReadIdx(0) {
    String continuing = "";
}

void RFD900x::init(uint32_t baud) {
  serial.begin(baud);
  instance = this;
  // RX interrupt is handled via serialEvent() on STM32 BlackPill
  delay(100);
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

void RFD900x::handleATReceiving(char buf[]) {
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

// ===== Buffer Management Functions =====

size_t RFD900x::readFromRxBuffer(uint8_t* buf, size_t maxLen) {
    size_t bytesRead = 0;
    
    while (bytesRead < maxLen && rxReadIdx != rxWriteIdx) {
        buf[bytesRead++] = rxBuffer[rxReadIdx];
        rxReadIdx = (rxReadIdx + 1) % RX_BUFFER_SIZE;
    }
    
    return bytesRead;
}

size_t RFD900x::getFirstPacketLength() {
    // Returns the length of the first complete packet including CR LF
    // Then use readFromRxBuffer to actually extract packets
    // Returns 0 if no complete packet found
    size_t currentIdx = rxReadIdx;
    size_t bytesToSearch = 0;
    // Calculate how many bytes are in the buffer
    if (rxWriteIdx >= rxReadIdx) {
        bytesToSearch = rxWriteIdx - rxReadIdx;
    } else {
        bytesToSearch = RX_BUFFER_SIZE - (rxReadIdx - rxWriteIdx);
    }
    // Need at least 2 bytes for CR LF
    if (bytesToSearch < 2) {
        return 0;  // Not enough data
    }
    // Search for CR LF sequence
    size_t searchCount = 0;
    while (searchCount < bytesToSearch - 1) {
        uint8_t currentByte = rxBuffer[currentIdx];
        uint8_t nextIdx = (currentIdx + 1) % RX_BUFFER_SIZE;
        uint8_t nextByte = rxBuffer[nextIdx];
        
        // Check for CR (\r = 0x0D) followed by LF (\n = 0x0A)
        if (currentByte == 0x0D && nextByte == 0x0A) {
            // Calculate length from rxReadIdx to after LF (including CR LF)
            size_t length;
            if (nextIdx >= rxReadIdx) {
                length = (nextIdx - rxReadIdx) + 1;  // +1 to include LF
            } else {
                // Packet wraps around circular buffer
                length = (RX_BUFFER_SIZE - rxReadIdx) + nextIdx + 1;
            }
            return length;
        }
        
        currentIdx = nextIdx;
        searchCount++;
    }
    
    return 0;  // No complete packet found
}


bool RFD900x::writeToTxBuffer(const uint8_t* pack, size_t len) {
    // Check if packet is too large
    if (len > MAX_PACKET_SIZE) {
        return false;
    }
    
    // Calculate if there's space for this packet
    size_t spaceNeeded = len;
    size_t currentUsage = getTxBufferUsage();
    
    // If buffer can't fit the packet, auto-flush first
    if (currentUsage + spaceNeeded > TX_BUFFER_SIZE) {
        flushTxBuffer();
    }
    
    // Write packet data sequentially
    for (size_t i = 0; i < len; i++) {
        txBuffer[txWriteIdx] = pack[i];
        txWriteIdx = (txWriteIdx + 1) % TX_BUFFER_SIZE;
    }
    
    return true;
}

void RFD900x::flushTxBuffer() {
    // Send all buffered data
    while (txReadIdx != txWriteIdx) {
        serial.write(txBuffer[txReadIdx]);
        txReadIdx = (txReadIdx + 1) % TX_BUFFER_SIZE;
    }
    serial.flush();
}

size_t RFD900x::getTxBufferUsage() {
    if (txWriteIdx >= txReadIdx) {
        return txWriteIdx - txReadIdx;
    } else {
        return TX_BUFFER_SIZE - (txReadIdx - txWriteIdx);
    }
}

// ===== ISR Handlers =====

void RFD900x::rxISR() {
    if (instance) {
        instance->handleRxByte((uint8_t)instance->serial.read());
    }
}

void RFD900x::handleRxByte(uint8_t byte) {
    size_t nextWriteIdx = (rxWriteIdx + 1) % RX_BUFFER_SIZE;
    
    // Only write if buffer is not full
    if (nextWriteIdx != rxReadIdx) {
        rxBuffer[rxWriteIdx] = byte;
        rxWriteIdx = nextWriteIdx;
    }
}

// STM32 BlackPill serialEvent - called automatically when serial data arrives
// This is the interrupt handler for the default USART
#ifdef STM32F411xE
extern void serialEvent() {
    if (RFD900x::instance) {
        RFD900x::rxISR();
    }
}
#endif