#include "spiFlash.h"
#include <string.h>

class spliFlashTester{
private:
    void printResult(const char* testName, bool passed) {
		Serial.print(testName);
		Serial.print(": ");
		Serial.println(passed ? "PASS" : "FAIL");
	}

    void testStartUp(){
        spiFlash spiFlash(256, 128);
        bool ok = spiFlash.startUp();
        printResult("StartUp test: ", ok);
    }

    void testReadWrite(){
        spiFlash spiFlash(256, 128);
        spiFlash.startUp();

        const char msg = "HELLO ROCKET TEAM!!";
        char readback[sizeof(msg)] = {0}; // open memory for our data

        ssize_t written = spiFlash.write(sizeof(msg), msg); // save what the flash interprets from our message
        ssize_t read = spiFlash.read(0, sizeof(msg), readback); // read the data into space we opened for it
        
        bool test = ((written == sizeof(msg)) && (read == sizeof(msg)) && !std::memcmp(written, read, sizeof(msg)));
        printResult("Basic Read Write test: ", ok);
    }

    void testBufferFlush() {
        spiFlash spiFlash(256, 128);
        spiFlash.startUp();

        const char msg[] = "BUFFERED";
        char out[sizeof(msg)] = {0};

        spiFlash.buffer(sizeof(msg), msg);
        spiFlash.flush();
        spiFlash.read(0, sizeof(msg), out);

        bool ok = !std::memcmp(msg, out, sizeof(msg));
        printResult("Buffer flush: ", ok);
    }

    void testQueuePriority() {
        spiFlash spiFlash(256, 128);
        spiFlash.startUp();

        char low[]  = "LOW";
        char high[] = "HIGH";

        spiFlash.queue(sizeof(low),  low,  spiFlash::P_UNIMPORTANT);
        spiFlash.queue(sizeof(high), high, spiFlash::P_URGENT);

        while (spiFlash.tick() > 0) {}

        char out1[5] = {0};
        char out2[4] = {0};

        spiFlash.read(0, sizeof(high), out1);
        spiFlash.read(sizeof(high), sizeof(low), out2);

        bool ok = (std::strcmp(out1, "HIGH") == 0 && std::strcmp(out2, "LOW")  == 0);
        printResult("Queue priority: ", ok);
    }


public:
    void run() {
		Serial.println("SPI Flash Tester: start");
        testStartup();
        testReadWrite();
        testBufferFlush();
        testQueuePriority();
		Serial.println("SPI Flash Tester: done");
	}
    ~spliFlashTester();

};

spliFlashTester:: spliFlashTester() = default;
spliFlashTester::~spliFlashTester() = default;
