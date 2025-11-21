#ifndef SPI_FLASH_H
#define SPI_FLASH_H

// #include <Arduino.h>
// #include <SPI.h>
// #include <SdFat.h>

// #include <Adafruit_SPIFlash.h>

#include <queue> 
#include <tuple>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>

//TODO: Documentation, Order methods in aphabetical
class spiFlash {
 public:
    //Constructor:
    spiFlash (const char cs_pin, const size_t buffer_size, const size_t k_buffer_size) ;

    //destructor:
    ~spiFlash() ;

    //Get Methods:
    char getCS_PIN();

    //Set Methods:
    void setCS_PIN(char pin);

    //functionality methods:
    ssize_t read(const size_t offset, const size_t bytes, char* buffer);

    char queue(size_t bytes, char* data, char priority = P_UNIMPORTANT);

    char buffer (const size_t bytes, const char* data);

    ssize_t write (const size_t bytes, const char* data);

    ssize_t kwrite (const size_t bytes, const char* data);

    char flush (void);

    ssize_t kLog (const size_t bytes, const char* data);

    char kflush (void);

    ssize_t tick(void) ;

    //types
    struct cmp_io_priority {
        bool operator()(const std::tuple<char, size_t, char*>& l, const std::tuple<char, size_t, char*>& r) const ;
    };

    static constexpr const char
        P_MANDATORY   = 0, //just force writes this at next tick
        P_URGENT      = 1,
        P_IMPORTANT   = 2,
        P_STD         = 3,
        P_UNIMPORTANT = 4,
        P_OPTIONAL    = 5
    ;

    const size_t   buffer_size;
    const size_t k_buffer_size;

 private:
    char CS_PIN; 
    
    std::priority_queue<
    //                         priority size    data
                    std::tuple<char, size_t, char*>,
        std::vector<std::tuple<char, size_t, char*>>,
        cmp_io_priority
    > queuedos;

    char* obuff;
    size_t buffer_offset;

    char* kbuff;
    size_t k_buffer_offset;

    int fd, kfd;
};

#endif