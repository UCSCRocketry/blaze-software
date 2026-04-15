#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <Arduino.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <queue>
#include <tuple>
#include <vector>

/** Stream each root-level file on SPI flash FAT (copy-out; flash is unchanged). */
struct SpiFlashExportCallbacks {
    void* user;
    bool (*onBeginFile)(void* user, const char* filename);
    bool (*onWrite)(void* user, const uint8_t* data, size_t len);
    bool (*onEndFile)(void* user);
};

class spiFlash {
 public:
    static constexpr char P_MANDATORY = 0;
    static constexpr char P_URGENT = 1;
    static constexpr char P_IMPORTANT = 2;
    static constexpr char P_STD = 3;
    static constexpr char P_UNIMPORTANT = 4;
    static constexpr char P_OPTIONAL = 5;

    spiFlash(const size_t buffer_size = 512, const size_t k_buffer_size = 512);

    ~spiFlash();

    bool startUp();

    uint8_t getCS_PIN();

    /** Copy payload into the priority queue; safe for stack buffers. Returns 0 or negative on error. */
    int queue(size_t bytes, const char* data, char priority = P_UNIMPORTANT);

    /** Process one (or one batch of mandatory) queued write(s). */
    ssize_t tick(void);

    /** Append to RAM buffer; spills to flash file when full. Returns 0 or negative on error. */
    int buffer(const size_t bytes, const char* data);

    /** Write buffered data file bytes to flash. Returns 0 or negative on error. */
    int flush(void);

    /** Append to log RAM buffer. */
    ssize_t kLog(const size_t bytes, const char* data);

    /** Write log buffer to flash log file. Returns 0 or negative on error. */
    int kflush(void);

    ssize_t kwrite(const size_t bytes, const char* data);

    ssize_t write(const size_t bytes, const char* data);

    /** Read from the data file at byte offset (uses global data file handle). */
    ssize_t read(const size_t offset, const size_t bytes, char* buffer);

    /**
     * Walk the FAT root (regular files only; no subdirectories). Flushes write buffers first.
     * SPI flash files are not modified or deleted.
     */
    bool exportRootFiles(const SpiFlashExportCallbacks* callbacks);

    /**
     * Delete a file on SPI flash by path (e.g. "DATA000.txt"). Uses FatVolume::remove.
     * Do not remove a path that is the same as the currently open data/log session file.
     */
    bool removeFile(const char* path);

    struct cmp_io_priority {
        bool operator()(const std::tuple<char, std::vector<char>>& l,
                        const std::tuple<char, std::vector<char>>& r) const;
    };

    const size_t buffer_size;
    const size_t k_buffer_size;

 private:
    std::priority_queue<std::tuple<char, std::vector<char>>,
                        std::vector<std::tuple<char, std::vector<char>>>,
                        cmp_io_priority>
        queuedos;

    char* obuff;
    size_t buffer_offset;

    char* kbuff;
    size_t k_buffer_offset;
};

#endif
