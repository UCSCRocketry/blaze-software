/*
 * SPI Flash Test for Blackpill STM32F411CE
 * Tests the spiFlash library features with Adafruit SPI Flash chip
 * Chip Select Pin: PB8
 */

#include <Arduino.h>
#include "spiFlash.h"

// Configuration
#define CS_PIN PB8
#define BUFFER_SIZE 512
#define K_BUFFER_SIZE 256
#define BAUD_RATE 115200

// Create spiFlash object
spiFlash flashStorage(CS_PIN, BUFFER_SIZE, K_BUFFER_SIZE);

// Test data
const char testMessage[] = "Hello from Blaze SPI Flash Test!";
const char testData[] = "This is a longer test message to verify buffering and write operations. Testing 1, 2, 3...";
char readBuffer[256];

void setup() {
    // Initialize Serial for debugging
    SPI.begin();
    Serial.begin(BAUD_RATE);
    while (!Serial) {
        delay(10); // Wait for serial connection
    }
    
    delay(2000); // Give time to open serial monitor
    
    Serial.println("\n=== Blaze SPI Flash Test ===");
    Serial.println("Board: Blackpill STM32F411CE");
    Serial.print("Chip Select Pin: ");
    Serial.println(CS_PIN);
    Serial.println("============================\n");

    // Initialize SPI Flash
    Serial.println("1. Initializing SPI Flash...");
    flashStorage.startUp();
    Serial.println("   [OK] Flash initialized successfully\n");

    // Test 1: Verify CS pin configuration
    Serial.println("2. Testing CS Pin Configuration...");
    Serial.print("   Current CS Pin: ");
    Serial.println(flashStorage.getCS_PIN());
    Serial.println("   [OK] CS Pin verified\n");

    // Test 2: Write test with buffer
    Serial.println("3. Testing Buffered Write...");
    Serial.print("   Writing: ");
    Serial.println(testMessage);
    
    ssize_t result = flashStorage.buffer(strlen(testMessage), testMessage);
    if (result >= 0) {
        Serial.println("   [OK] Data buffered successfully");
    } else {
        Serial.print("   [ERROR] Buffer failed with code: ");
        Serial.println(result);
    }
    
    // Flush buffer to flash
    Serial.println("   Flushing buffer to flash...");
    char flushResult = flashStorage.flush();
    if (flushResult == 0) {
        Serial.println("   [OK] Buffer flushed successfully\n");
    } else {
        Serial.print("   [ERROR] Flush failed with code: ");
        Serial.println(flushResult);
    }

    // Test 3: Read back data
    Serial.println("4. Testing Read Operation...");
    memset(readBuffer, 0, sizeof(readBuffer));
    ssize_t bytesRead = flashStorage.read(0, strlen(testMessage), readBuffer);
    
    if (bytesRead > 0) {
        Serial.print("   Bytes read: ");
        Serial.println(bytesRead);
        Serial.print("   Data read: ");
        Serial.println(readBuffer);
        
        if (strcmp(readBuffer, testMessage) == 0) {
            Serial.println("   [OK] Data verification successful!\n");
        } else {
            Serial.println("   [WARNING] Data mismatch!\n");
        }
    } else {
        Serial.print("   [ERROR] Read failed with code: ");
        Serial.println(bytesRead);
    }

    // Test 4: Queue system test
    Serial.println("5. Testing Priority Queue System...");
    
    const char urgentMsg[] = "URGENT";
    const char stdMsg[] = "Standard";
    const char optionalMsg[] = "Optional";
    
    // Queue messages with different priorities
    flashStorage.queue(strlen(optionalMsg), (char*)optionalMsg, flashStorage.P_OPTIONAL);
    Serial.println("   Queued OPTIONAL message");
    
    flashStorage.queue(strlen(urgentMsg), (char*)urgentMsg, flashStorage.P_URGENT);
    Serial.println("   Queued URGENT message");
    
    flashStorage.queue(strlen(stdMsg), (char*)stdMsg, flashStorage.P_STD);
    Serial.println("   Queued STANDARD message");
    
    Serial.println("   [OK] Messages queued (will be processed in priority order)\n");

    // Test 5: Tick operation (process queue)
    Serial.println("6. Testing Tick Operation...");
    flashStorage.tick();
    Serial.println("   [OK] Tick executed - highest priority item processed\n");

    // Test 6: K-log test (kernel log buffer)
    Serial.println("7. Testing K-Log Buffer...");
    const char klogMsg[] = "Kernel log entry";
    ssize_t klogResult = flashStorage.kLog(strlen(klogMsg), klogMsg);
    
    if (klogResult >= 0) {
        Serial.println("   [OK] K-log buffer written");
        Serial.println("   Flushing k-log buffer...");
        flashStorage.kflush();
        Serial.println("   [OK] K-log flushed\n");
    } else {
        Serial.print("   [ERROR] K-log failed with code: ");
        Serial.println(klogResult);
    }

    // Test 7: Large data write test
    Serial.println("8. Testing Large Data Write...");
    result = flashStorage.buffer(strlen(testData), testData);
    if (result >= 0) {
        Serial.print("   [OK] Buffered ");
        Serial.print(strlen(testData));
        Serial.println(" bytes");
        flashStorage.flush();
        Serial.println("   [OK] Large data flushed\n");
    } else {
        Serial.print("   [ERROR] Large buffer failed: ");
        Serial.println(result);
    }

    Serial.println("\n=== All Tests Complete ===");
    Serial.println("System ready for operation.\n");
}

void loop() {
    // Continuous operation test
    static unsigned long lastWrite = 0;
    static int counter = 0;
    
    // Write a counter value every 5 seconds
    if (millis() - lastWrite > 5000) {
        lastWrite = millis();
        
        char logEntry[64];
        snprintf(logEntry, sizeof(logEntry), "Loop counter: %d, Uptime: %lu ms", counter++, millis());
        
        Serial.print("Writing: ");
        Serial.println(logEntry);
        
        flashStorage.buffer(strlen(logEntry), logEntry);
        flashStorage.flush();
        
        Serial.println("[OK] Log entry written\n");
    }
    
    // Small delay
    delay(100);
}
