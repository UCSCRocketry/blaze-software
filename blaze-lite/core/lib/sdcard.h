
#if defined(_WIN32) || defined(_WIN64)
  #ifndef SD_CARD_H
  #define SD_CARD_H
  #include <queue> 
  #include <tuple>
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include <cmath>
  #include <vector>
  #include <fstream>
  #include <iostream>
  typedef std::ptrdiff_t ssize_t;
  #else
  #include <sys/types.h>
#endif
struct cmp_io_priority {
    bool operator()(const std::tuple<char, size_t, char*>& a, const std::tuple<char, size_t, char*>& b) {
        return std::get<0>(a) > std::get<0>(b); // Lower priority value means higher priority
    }
};


class sdCard {
    public :
        sdCard(uint8_t cs_pin = 3, size_t buf_size = 512, size_t k_buf_size = 2048);
        ~sdCard();
        void init();
        bool isInitialized();
        void readFile(const char* filename);

       //Get Methods:
        uint8_t getCS_PIN() {
            return CS_PIN;
        };

        //Set Methods:
        void setCS_PIN(uint8_t pin) {
            CS_PIN = pin;
        };

        //functionality methods 
        ssize_t read(const size_t offset, const size_t bytes, char* buffer);
        ssize_t write(const size_t bytes, const char* data);
        char queue(size_t bytes, char* data, char priority = P_UNIMPORTANT);
        char buffer (const size_t bytes, const char* data);
        ssize_t kwrite (const size_t bytes, const char* data);
        char flush (void);
        ssize_t kLog(const size_t bytes, const char* data);
        char kflush (void);
        ssize_t tick (void);

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
struct BarometerReading {
    unsigned long timestamp;
    float pressure;
    float altitude;
    float temperature;
};
#endif // SD_CARD_H
