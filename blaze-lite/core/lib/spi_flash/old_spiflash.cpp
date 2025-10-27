#include "spiflash.hpp"

#include <queue>
#include <tuple>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <functional>


/* constexpr */ const size_t spiflash::buffer_size = 4098;
size_t spiflash::buffer_offset = 0;

char spiflash::obuff[/* spiflash::buffer_size */ 4098];

bool spiflash::CmpOPriority::operator()(const std::tuple<uint8_t, size_t, char*>& l, const std::tuple<uint8_t, size_t, char*>& r) const {
    return std::get<0>(l) > std::get<0>(r);
}


    std::priority_queue<
                    std::tuple<uint8_t, size_t, char*>,
        std::vector<std::tuple<uint8_t, size_t, char*>>,
        spiflash::CmpOPriority
    > spiflash::queuedos = std::priority_queue<
                    std::tuple<uint8_t, size_t, char*>,
        std::vector<std::tuple<uint8_t, size_t, char*>>,
        spiflash::CmpOPriority
    >(); //todo: do I need to make this atomic?

// priorities from most to least important
/* constexpr */ const uint8_t
    spiflash::P_MANDATORY   = 0, //just force writes this at next tick
    spiflash::P_URGENT      = 1,
    spiflash::P_IMPORTANT   = 2,
    spiflash::P_STD         = 3,
    spiflash::P_UNIMPORTANT = 4,
    spiflash::P_OPTIONAL    = 5
;

ssize_t spiflash:: read (const size_t offset, const size_t bytes, char* buffer)  ; //hard read. used for extracting data post-flight
ssize_t spiflash::kread (const size_t offset, const size_t bytes, char* buffer)  ; //hard read. used for extracting data post-flight

void spiflash::queue (/* const */ size_t bytes, /* const */ char* data, /* const */ uint8_t priority) {
    spiflash::queuedos.push(std::tie<uint8_t, size_t, char*>(priority, bytes, data));
}

ssize_t spiflash::buffer (const size_t bytes, const char* data) { //TODO: add err handeling
    size_t offset = 0, temp = 0;
    
    memcpy(spiflash::obuff + spiflash::buffer_offset, data, temp = std::min(bytes, spiflash::buffer_size - spiflash::buffer_offset));
    spiflash::buffer_offset += (offset += temp);

    while (bytes - offset > 0) {
        spiflash::flush();
        memcpy(spiflash::obuff, data + offset, temp = std::min(bytes - offset, spiflash::buffer_size - spiflash::buffer_offset));
        spiflash::buffer_offset += temp;
        offset += temp;
    }

    return offset;
}

ssize_t spiflash::write (const size_t bytes, const char* data) {
    //TODO MAKE REAL, rn is just a testr function
    
    // static auto fd = 0;
    // if(!fd) fd = open("./spiflashTestDump.txt", O_CREAT | O_RDWR);

    // write(bytes, data); //FIXME: this is supposed to be normal write function

    std::cout.write(data, bytes);
    return 0;
}

void spiflash::flush (void) {
    spiflash::write (spiflash::buffer_offset, spiflash::obuff);
    memset(spiflash::obuff, 0, spiflash::buffer_size);
    spiflash::buffer_offset = 0;
}

size_t spiflash::kLog   (const size_t bytes, const char* data) ; //just stright up a write
size_t spiflash::kWrite (const size_t bytes, const char* data) ; //just stright up a write
size_t spiflash::kFlush (const size_t bytes, const char* data) ; //just stright up a write

int  spiflash::tick (void) ;