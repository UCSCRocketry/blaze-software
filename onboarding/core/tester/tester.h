#ifndef TESTER_H
#define TESTER_H

#include <Arduino.h>

class tester {
    public:
        //Constructor
        tester();

    //Test functions
    bool protocolTest(const uint8_t* inComingPacket, size_t packetLength);

    //TO-DO: function to test the math for state estimation
};

#endif