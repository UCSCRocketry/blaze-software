/*
    * spiFlash.cpp - SPI Flash library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/
#include "spiFlash.h"

#include <queue> 
#include <tuple>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <fcntl.h>

//TODO: Make private methods to simplify code
/*
    //char getCS_PIN(char pin);

    //void setCS_PIN(char pin);

    ssize_t read(const size_t offset, const size_t bytes, const char* buffer);

    //char queue(size_t bytes, char* data, int priority = P_UNIMPORTANT);

    //char buffer (const size_t bytes, char* data);

    ssize_t write (const size_t bytes, const char* data);

    //ssize_t kwrite (const size_t bytes, const char* data); 

    //void flush (void);

    //ssize_t kLog (const size_t bytes, const char* data); //buffer but with k stuff

    //void kflush (void); //flush but for k stuff

    ssize_t tick(void) ;
*/
//Constructor:
spiFlash::spiFlash (const char cs_pin, const size_t buffer_size, const size_t k_buffer_size) : CS_PIN(cs_pin), buffer_size(buffer_size), k_buffer_size(k_buffer_size), buffer_offset(0), k_buffer_offset(0) {
    obuff = new char[  buffer_size];
    kbuff = new char[k_buffer_size];
    queuedos = std::priority_queue<std::tuple<char, size_t, char*>, std::vector<std::tuple<char, size_t, char*>>, cmp_io_priority>();
    
    fd  = open("./spifTestNormal.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //TODO: remove once write() works
    kfd = open("./spifTestKernel.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

spiFlash::~spiFlash () {
    kflush();
    flush();

    close(fd);
    close(kfd);

    delete[] obuff;
    delete[] kbuff;
}

//Get Methods:
char spiFlash::getCS_PIN() { return CS_PIN; }

//Set Methods:
void spiFlash::setCS_PIN(const char pin) { CS_PIN = pin; }

//functionality methods:
ssize_t spiFlash::read(const size_t offset, const size_t bytes, char* buffer){
    //error handling
    if (bytes == 0) return 0; //no bytes to read
    if (buffer == nullptr) return -1; //null data pointer
    if (lseek(fd, offset, SEEK_SET) == -1 ) return -2; //seek error

    ssize_t bytes_read = ::read(fd, buffer, bytes);

    return bytes_read;
}

char spiFlash::queue(size_t bytes, char* data, char priority) {
    queuedos.push(std::tie<char, size_t, char*>(priority, bytes, data));
    return 0;
}

char spiFlash::buffer (const size_t bytes, const char* data) {
    //function call error handling
    if (bytes == 0) return 0; //no bytes to write
    if (data == nullptr) return -1; //null data pointer
    if (obuff == nullptr) return -2; //null buffer pointer

    ssize_t err = 0;
    size_t offset = 0, temp = 0;
    
    memcpy(obuff + buffer_offset, data, temp = std::min(bytes, buffer_size - buffer_offset));
    buffer_offset += (offset += temp);

    while (bytes - offset > 0) {
        if (err = flush() < 0) return err;
        flush();
        
        memcpy(obuff, data + offset, temp = std::min(bytes - offset, buffer_size));
        buffer_offset += temp;
        offset += temp;
    }

    return err;
}

//return value < 0 means error
ssize_t spiFlash::write (const size_t bytes, const char* data) {
    return ::write(fd, data, bytes);
}

ssize_t spiFlash::kwrite (const size_t bytes, const char* data) {
    return ::write(kfd, data, bytes);
}

char spiFlash::flush (void) {
    ssize_t err = write (buffer_offset, obuff);
    if (err < 0) return err;
    
    memset(obuff, 0, buffer_size);
    buffer_offset = 0;
    
    return 0;
}

ssize_t spiFlash::kLog (const size_t bytes, const char* data) {
    //function call error handling
    if (bytes == 0) return 0; //no bytes to write
    if (data == nullptr) return -1; //null data pointer
    if (kbuff == nullptr) return -2; //null buffer pointer

    ssize_t err = 0;
    size_t offset = 0, temp = 0;
    
    memcpy(kbuff + k_buffer_offset, data, temp = std::min(bytes, k_buffer_size - k_buffer_offset));
    k_buffer_offset += (offset += temp);

    while (bytes - offset > 0) {
        if (err = kflush() < 0) return err;
        memcpy(kbuff, data + offset, temp = std::min(bytes - offset, k_buffer_size - k_buffer_offset));
        k_buffer_offset += temp;
        offset += temp;
    }

    return err;
}

char spiFlash::kflush (void) {
    ssize_t err = kwrite (k_buffer_offset, kbuff);
    if (err < 0) return err;
    memset(kbuff, 0, k_buffer_size);
    k_buffer_offset = 0;
    return err;
}

bool spiFlash::cmp_io_priority:: operator()(const std::tuple<char, size_t, char*>& l, const std::tuple<char, size_t, char*>& r) const {
        return std::get<0>(l) > std::get<0>(r);
}

// constexpr const char
//     spiFlash::P_MANDATORY   = 0, //just force writes this at next tick
//     spiFlash::P_URGENT      = 1,
//     spiFlash::P_IMPORTANT   = 2,
//     spiFlash::P_STD         = 3,
//     spiFlash::P_UNIMPORTANT = 4,
//     spiFlash::P_OPTIONAL    = 5
// ;
