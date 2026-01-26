#include "sdCard.h"

#include <string.h>

#define MSG1 "This is the first msg."
#define MSG2 "This is the second msg."

#define RDBUF_SIZE 256
char readBuff[RDBUF_SIZE];

void setup() {
    Serial.begin(9600); // open the serial port at 9600 bps:
    delay(5000); // Wait for serial connection to establish
    
    Serial.println("Starting SD card test...");
    
    sdCard sd(PB15); // CS_SD is on PB15
    sd.startUp(); // Initialize the SD card
    int buf;
    memset (readBuff, 0, RDBUF_SIZE * sizeof(char));
    
    Serial.print("csPin: ");
    Serial.print(sd.getCS_PIN());
    Serial.print('\n');

    //test write
    if ((buf = sd.writeData(sizeof(MSG1), MSG1)) < -1) {
        Serial.print("writeData failed: MSG1");
        return;
    }
    Serial.print("WriteData wrote ");
    Serial.print(buf);
    Serial.print("bytes of MSG1.\n");
    // Serial.print("The message it wrote is:\n  hex:"); //No access to the acctual files :sob:

    if ((buf = sd.writeData(sizeof(MSG2), MSG2)) < -1) {
        Serial.print("writeData failed: MSG2");
        return;
    }
    Serial.print("WriteData wrote ");
    Serial.print(buf);
    Serial.print("bytes of MSG2.\n");

    //test read
    if ((buf = sd.readData(sizeof(MSG1), readBuff)) < -1) {
        Serial.print("readData failed: MSG1");
        return;
    }
    Serial.print("ReadData read ");
    Serial.print(buf);
    Serial.print("bytes of MSG1.\n");
    Serial.print("The message it read is:\n  hex:");
    for (int i = 0; i < buf; ++i) {
        Serial.print(readBuff[i], HEX);
        Serial.print("  ");
    }
    Serial.print("\n  Text: \"");
    Serial.print(readBuff); //assuming it can handel null termianted strings
    Serial.print("\"\n");

    if ((buf = sd.readData(sizeof(MSG2), readBuff)) < -1) {
        Serial.print("readData failed: MSG2");
        return;
    }
    Serial.print("ReadData read ");
    Serial.print(buf);
    Serial.print("bytes of MSG2.\n");
    Serial.print("The message it read is:\n  hex:");
    for (int i = 0; i < buf; ++i) {
        Serial.print(readBuff[i], HEX);
        Serial.print("  ");
    }
    Serial.print("\n  Text: \"");
    Serial.print(readBuff); //assuming it can handel null termianted strings
    Serial.print("\"\n");

    //log

    //test write
    if ((buf = sd.writeLog(MSG1, sizeof(MSG1))) < -1) {
        Serial.print("writeLog failed: MSG1");
        return;
    }
    Serial.print("WriteLog wrote ");
    Serial.print(buf);
    Serial.print("bytes of MSG1.\n");
    // Serial.print("The message it wrote is:\n  hex:"); //No access to the acctual files :sob:

    if ((buf = sd.writeLog(MSG2, sizeof(MSG2))) < -1) {
        Serial.print("writeLog failed: MSG2");
        return;
    }
    Serial.print("WriteLog wrote ");
    Serial.print(buf);
    Serial.print("bytes of MSG2.\n");

    //test read
    if ((buf = sd.readLog(readBuff, sizeof(MSG1))) < -1) {
        Serial.print("readLog failed: MSG1");
        return;
    }
    Serial.print("ReadLog read ");
    Serial.print(buf);
    Serial.print("bytes of MSG1.\n");
    Serial.print("The message it read is:\n  hex:");
    for (int i = 0; i < buf; ++i) {
        Serial.print(readBuff[i], HEX);
        Serial.print("  ");
    }
    Serial.print("\n  Text: \"");
    Serial.print(readBuff); //assuming it can handel null termianted strings
    Serial.print("\"\n");

    if ((buf = sd.readLog(readBuff, sizeof(MSG2))) < -1) {
        Serial.print("readLog failed: MSG2");
        return;
    }
    Serial.print("ReadLog read ");
    Serial.print(buf);
    Serial.print("bytes of MSG2.\n");
    Serial.print("The message it read is:\n  hex:");
    for (int i = 0; i < buf; ++i) {
        Serial.print(readBuff[i], HEX);
        Serial.print("  ");
    }
    Serial.print("\n  Text: \"");
    Serial.print(readBuff); //assuming it can handel null termianted strings
    Serial.print("\"\n");

    //tests passed
    Serial.print("\nAll Tests Passed");
}

void loop () {
    //
}