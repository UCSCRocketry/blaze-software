
#include <Arduino.h>

#include "dataPacket.h"

class DataPacketTester {
public:
	void run() {
		Serial.println("DataPacket Tester: start");
		testEncodeDecode();
		testInvalidEndBytes();
		Serial.println("DataPacket Tester: done");
	}

private:
	void printResult(const char* testName, bool passed) {
		Serial.print(testName);
		Serial.print(": ");
		Serial.println(passed ? "PASS" : "FAIL");
	}

	void testEncodeDecode() {
		DataPacket packet(StartByte::EXPECT_ACK);
		uint8_t payload[DataPacket::PAYLOAD_SIZE] = {
			'H','E','L','L','O',' ','R','O','C','K','E','T','S','!','!','!','!'
		};
		packet.encodePacket(payload, 'a', 'b');

		printPacketHex(packet.getBuffer(), packet.getLength());

		uint8_t* buffer = packet.getBuffer();
		bool digitsOk = true;
		for (size_t i = 1; i < 5; i++) {
			if (buffer[i] > 9) {
				digitsOk = false;
				break;
			}
		}

		DecodedPacket decoded{};
		bool ok = packet.decodePacket(packet.getBuffer(), packet.getLength(), decoded);
		bool payloadMatch = true;
		for (size_t i = 0; i < DataPacket::PAYLOAD_SIZE; i++) {
			if (decoded.payload[i] != payload[i]) {
				payloadMatch = false;
				break;
			}
		}

		bool passed = ok && decoded.isValid && digitsOk &&
					  decoded.startByte == StartByte::EXPECT_ACK &&
					  decoded.sequenceID == 0 &&
			      decoded.idA == 'a' && decoded.idB == 'b' &&
					  payloadMatch;

		printResult("Encode/Decode round-trip", passed);
	}

	void printPacketHex(const uint8_t* data, size_t len) {
		Serial.print("Packet hex: ");
		for (size_t i = 0; i < len; i++) {
			if (data[i] < 0x10) {
				Serial.print('0');
			}
			Serial.print(data[i], HEX);
			if (i + 1 < len) {
				Serial.print(' ');
			}
		}
		Serial.println();
	}

	void testInvalidEndBytes() {
		DataPacket packet(StartByte::ACK_RESPONSE);
		uint8_t payload[DataPacket::PAYLOAD_SIZE] = {0};
		packet.encodePacket(payload, 'X', 'Y');

		uint8_t* buffer = packet.getBuffer();
		size_t len = packet.getLength();
		buffer[len - 2] = 0x00;
		buffer[len - 1] = 0x00;

		DecodedPacket decoded{};
		bool ok = packet.decodePacket(buffer, len, decoded);
		printResult("Invalid end bytes", !ok && !decoded.isValid);
	}
};

void runDataPacketTests() {
	DataPacketTester tester;
	tester.run();
}
