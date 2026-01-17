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

    return false;
  }

  bool freqSet = radio.setFrequency(freq);

  if(!freqSet) {
    Serial.println("Failed to set frequency!");

    return false;
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



/*

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

*/