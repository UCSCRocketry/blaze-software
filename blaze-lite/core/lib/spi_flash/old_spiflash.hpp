/*
TODO:
- make amny paramerters const T&
*/

#pragma once
#include <queue>
#include <tuple>
#include <stdint.h>
#include <unistd.h>
#include <vector>

//common storage interface
#ifndef strg
    #define strg spiflash
#endif

#ifndef KLOG_SIZE
    #define KLOG_SIZE 4098
#endif

namespace spiflash {
    extern /* constexpr */ const size_t buffer_size;
    extern size_t buffer_offset;
    
    extern char obuff[/* buffer_size */ 4098];

    struct CmpOPriority {
        bool operator()(const std::tuple<uint8_t, size_t, char*>& l, const std::tuple<uint8_t, size_t, char*>& r) const ;
    };

    extern std::priority_queue<
                    std::tuple<uint8_t, size_t, char*>,
        std::vector<std::tuple<uint8_t, size_t, char*>>,
        CmpOPriority
    > queuedos; //todo: do I need to make this atomic?

    //priorities from most to least important
    extern /* constexpr */ const uint8_t
        P_MANDATORY   , //just force writes this at next tick
        P_URGENT      ,
        P_IMPORTANT   ,
        P_STD         ,
        P_UNIMPORTANT ,
        P_OPTIONAL    
    ;

    ssize_t  read (const size_t offset, const size_t bytes, char* buffer)  ;
    ssize_t kread (const size_t offset, const size_t bytes, char* buffer)  ;

    void queue (/* const */ size_t bytes, /* const */ char* data, /* const */ uint8_t priority = P_STD /*std priority*/) ;

    ssize_t buffer (const size_t bytes, const char* data) ;

    ssize_t write (const size_t bytes, const char* data) ;

    void flush (void) ;

    size_t kLog   (const size_t bytes, const char* data) ; //just stright up a write
    size_t kWrite (const size_t bytes, const char* data) ; //just stright up a write
    size_t kFlush (const size_t bytes, const char* data) ; //just stright up a write

    int  tick (void) ;
}