/*
    * spiFlash.cpp - SPI Flash library for Blaze
    * LittleFS version
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/
#include "spiFlash.h"

#include <SPI.h>

#include <Adafruit_SPIFlash.h>
#include <flash_devices.h>

extern "C" {
#include "lfs.h"
}

#include <algorithm>
#include <cstdio>
#include <cstring>

// Set this to 0 in platformio.ini if you do not want a mount failure to erase/reformat flash:
#ifndef SPI_FLASH_AUTO_FORMAT_ON_MOUNT_FAIL
#define SPI_FLASH_AUTO_FORMAT_ON_MOUNT_FAIL 1
#endif

namespace {

const uint8_t kFlashCsPin = PB8;

// W25Q128 / GD25Q128 style SPI NOR flash geometry.
static constexpr lfs_size_t kLfsReadSize = 16;
static constexpr lfs_size_t kLfsProgSize = 256;
static constexpr lfs_size_t kLfsBlockSize = 4096;
static constexpr lfs_size_t kLfsCacheSize = 256;
static constexpr lfs_size_t kLfsLookaheadSize = 128;
static constexpr int32_t kLfsBlockCycles = 500;

// GD25Q128 (16 MiB): same ID pattern as GD25Q64 but capacity 0x18 — not in Adafruit's default table.
static const SPIFlash_Device_t kGd25q128 = {
    .total_size = (1UL << 24),
    .start_up_time_us = 5000,
    .manufacturer_id = 0xc8,
    .memory_type = 0x40,
    .capacity = 0x18,
    .max_clock_speed_mhz = 104,
    .quad_enable_bit_mask = 0x02,
    .has_sector_protection = false,
    .supports_fast_read = true,
    .supports_qspi = true,
    .supports_qspi_writes = true,
    .write_status_register_split = true,
    .single_status_byte = false,
    .is_fram = false,
};

// Order: try exact parts used on Blaze / common alternates before library defaults.
static const SPIFlash_Device_t kBlazeFlashCandidates[] = {
    W25Q128JV_SQ,
    W25Q128JV_PM,
    MX25L12833F,
    kGd25q128,
};

void logRawJedec(uint8_t csPin) {
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    delayMicroseconds(50);
    SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
    delayMicroseconds(2);
    SPI.transfer(0x9F);
    uint8_t m = SPI.transfer(0xFF);
    uint8_t t = SPI.transfer(0xFF);
    uint8_t c = SPI.transfer(0xFF);
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
    Serial.print("SPI flash raw JEDEC (manuf, type, cap): 0x");
    if (m < 16) {
        Serial.print('0');
    }
    Serial.print(m, HEX);
    Serial.print(" 0x");
    if (t < 16) {
        Serial.print('0');
    }
    Serial.print(t, HEX);
    Serial.print(" 0x");
    if (c < 16) {
        Serial.print('0');
    }
    Serial.print(c, HEX);
    Serial.println(" (if FF FF FF check wiring / CS pin / power)");
}

Adafruit_FlashTransport_SPI flashTransport(kFlashCsPin, SPI);
Adafruit_SPIFlash flashChip(&flashTransport);

lfs_t littlefs;
lfs_config lfsConfig;
lfs_file_t dataFile;
lfs_file_t logFile;

bool lfsConfigured = false;
bool fsMounted = false;
bool dataFileOpen = false;
bool logFileOpen = false;

char dataFileName[16] = {0};
char logFileName[16] = {0};

int lfsReadCb(const struct lfs_config* c,
              lfs_block_t block,
              lfs_off_t off,
              void* buffer,
              lfs_size_t size) {
    const uint32_t address = static_cast<uint32_t>(block * c->block_size + off);
    return flashChip.readBuffer(address, static_cast<uint8_t*>(buffer), size) ? 0 : LFS_ERR_IO;
}

int lfsProgCb(const struct lfs_config* c,
              lfs_block_t block,
              lfs_off_t off,
              const void* buffer,
              lfs_size_t size) {
    const uint32_t address = static_cast<uint32_t>(block * c->block_size + off);
    return flashChip.writeBuffer(address, static_cast<const uint8_t*>(buffer), size) ? 0 : LFS_ERR_IO;
}

int lfsEraseCb(const struct lfs_config* /*c*/, lfs_block_t block) {
    // LittleFS block size is 4096, so the LittleFS block number maps directly to a 4 KiB flash sector.
    return flashChip.eraseSector(block) ? 0 : LFS_ERR_IO;
}

int lfsSyncCb(const struct lfs_config* /*c*/) {
    // Adafruit_SPIFlash operations are blocking; wait here for compatibility with LittleFS sync.
    flashChip.waitUntilReady();
    return 0;
}

void configureLittleFs() {
    memset(&lfsConfig, 0, sizeof(lfsConfig));

    lfsConfig.read = lfsReadCb;
    lfsConfig.prog = lfsProgCb;
    lfsConfig.erase = lfsEraseCb;
    lfsConfig.sync = lfsSyncCb;

    lfsConfig.read_size = kLfsReadSize;
    lfsConfig.prog_size = kLfsProgSize;
    lfsConfig.block_size = kLfsBlockSize;
    lfsConfig.block_count = flashChip.size() / kLfsBlockSize;
    lfsConfig.cache_size = kLfsCacheSize;
    lfsConfig.lookahead_size = kLfsLookaheadSize;
    lfsConfig.block_cycles = kLfsBlockCycles;

    lfsConfigured = true;
}

bool fileExists(const char* path) {
    lfs_info info;
    return lfs_stat(&littlefs, path, &info) == 0;
}

bool makeDataFileName(char* buffer, size_t bufferSize) {
    for (uint16_t fileIndex = 0; fileIndex < 1000; ++fileIndex) {
        const int n = snprintf(buffer, bufferSize, "DATA%03u.txt", fileIndex);
        if (n < 0 || static_cast<size_t>(n) >= bufferSize) {
            return false;
        }
        if (!fileExists(buffer)) {
            return true;
        }
    }
    return false;
}

bool makeLogFileName(char* buffer, size_t bufferSize) {
    for (uint16_t fileIndex = 0; fileIndex < 1000; ++fileIndex) {
        const int n = snprintf(buffer, bufferSize, "LOG%03u.txt", fileIndex);
        if (n < 0 || static_cast<size_t>(n) >= bufferSize) {
            return false;
        }
        if (!fileExists(buffer)) {
            return true;
        }
    }
    return false;
}

void printRootFiles() {
    lfs_dir_t dir;
    int err = lfs_dir_open(&littlefs, &dir, "/");
    if (err < 0) {
        Serial.print("LittleFS ls failed, error ");
        Serial.println(err);
        return;
    }

    Serial.println("LittleFS root files:");

    lfs_info info;
    while ((err = lfs_dir_read(&littlefs, &dir, &info)) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }

        Serial.print(info.type == LFS_TYPE_DIR ? "  <DIR>  " : "  <FILE> ");
        Serial.print(info.name);
        if (info.type == LFS_TYPE_REG) {
            Serial.print("  ");
            Serial.print(info.size);
            Serial.print(" bytes");
        }
        Serial.println();
    }

    if (err < 0) {
        Serial.print("LittleFS ls read failed, error ");
        Serial.println(err);
    }

    lfs_dir_close(&littlefs, &dir);
}

void closeOpenFiles() {
    if (logFileOpen) {
        lfs_file_close(&littlefs, &logFile);
        logFileOpen = false;
    }
    if (dataFileOpen) {
        lfs_file_close(&littlefs, &dataFile);
        dataFileOpen = false;
    }
}

}  // namespace

spiFlash::spiFlash(const size_t buffer_size, const size_t k_buffer_size)
    : buffer_size(buffer_size),
      k_buffer_size(k_buffer_size),
      buffer_offset(0),
      k_buffer_offset(0) {
    obuff = new char[buffer_size];
    kbuff = new char[k_buffer_size];
}

spiFlash::~spiFlash() {
    flush();
    kflush();
    closeOpenFiles();
    if (fsMounted) {
        lfs_unmount(&littlefs);
        fsMounted = false;
    }
    delete[] obuff;
    delete[] kbuff;
}

bool spiFlash::startUp() {
    // Pass explicit candidates: W25Q128JV-PM/IM (EF 70 18) is omitted from Adafruit's built-in list,
    // which only includes W25Q128JV-SQ (EF 40 18).
    Serial.println("Before SPI flash initialization:");
    if (!flashChip.begin(kBlazeFlashCandidates,
                         sizeof(kBlazeFlashCandidates) / sizeof(kBlazeFlashCandidates[0]))) {
        Serial.println("Error, failed to initialize flash chip!");
        logRawJedec(kFlashCsPin);
        return false;
    }
    Serial.print("Flash chip JEDEC ID: 0x");
    Serial.println(flashChip.getJEDECID(), HEX);

    configureLittleFs();

    if (!mountfs()) {
        Serial.println("Error, failed to mount LittleFS on SPI flash!");

#if SPI_FLASH_AUTO_FORMAT_ON_MOUNT_FAIL
        Serial.println("Attempting to format SPI flash as LittleFS...");

        closeOpenFiles();

        int err = lfs_format(&littlefs, &lfsConfig);
        if (err < 0) {
            Serial.print("Error, failed to format SPI flash LittleFS, error ");
            Serial.println(err);
            return false;
        }

        if (!mountfs()) {
            Serial.println("Error, format completed but LittleFS remount failed!");
            return false;
        }
        Serial.println("Formatted and mounted LittleFS filesystem.");
#else
        Serial.println("Auto-format disabled. Define SPI_FLASH_AUTO_FORMAT_ON_MOUNT_FAIL=1 to format.");
        return false;
#endif
    } else {
        Serial.println("Mounted LittleFS filesystem.");
    }

    if (!makeDataFileName(dataFileName, sizeof(dataFileName))) {
        Serial.println("Failed to allocate SPI flash data file name");
        return false;
    }
    if (!makeLogFileName(logFileName, sizeof(logFileName))) {
        Serial.println("Failed to allocate SPI flash log file name");
        return false;
    }

    closeOpenFiles();

    int err = lfs_file_open(&littlefs, &dataFile, dataFileName, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND);
    if (err < 0) {
        Serial.print("Failed to open SPI flash data file, error ");
        Serial.println(err);
        return false;
    }
    dataFileOpen = true;

    err = lfs_file_open(&littlefs, &logFile, logFileName, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND);
    if (err < 0) {
        Serial.print("Failed to open SPI flash log file, error ");
        Serial.println(err);
        lfs_file_close(&littlefs, &dataFile);
        dataFileOpen = false;
        return false;
    }
    logFileOpen = true;

    delay(500);

    printRootFiles();

    Serial.print("SPI flash data file: ");
    Serial.println(dataFileName);

    Serial.print("SPI flash log file: ");
    Serial.println(logFileName);

    return true;
}

uint8_t spiFlash::getCS_PIN() { return kFlashCsPin; }

ssize_t spiFlash::read(const size_t offset, const size_t bytes, char* buffer) {
    if (bytes == 0) {
        return 0;
    }
    if (buffer == nullptr) {
        return -1;
    }
    if (!fsMounted || !dataFileOpen) {
        return -2;
    }

    const lfs_soff_t seekResult = lfs_file_seek(&littlefs, &dataFile, static_cast<lfs_soff_t>(offset), LFS_SEEK_SET);
    if (seekResult < 0) {
        return -3;
    }

    const lfs_ssize_t n = lfs_file_read(&littlefs, &dataFile, buffer, static_cast<lfs_size_t>(bytes));
    if (n < 0) {
        return -4;
    }
    return static_cast<ssize_t>(n);
}

int spiFlash::queue(size_t bytes, const char* data, char priority) {
    if (bytes == 0) {
        return 0;
    }
    if (data == nullptr) {
        return -1;
    }
    std::vector<char> copy(data, data + bytes);
    queuedos.push(std::make_tuple(priority, std::move(copy)));
    return 0;
}

int spiFlash::buffer(const size_t bytes, const char* data) {
    if (bytes == 0) {
        return 0;
    }
    if (data == nullptr) {
        return -1;
    }
    if (obuff == nullptr) {
        return -2;
    }

    int err = 0;
    size_t offset = 0;
    size_t temp = 0;

    temp = std::min(bytes, buffer_size - buffer_offset);
    memcpy(obuff + buffer_offset, data, temp);
    buffer_offset += temp;
    offset += temp;

    while (bytes > offset && err == 0) {
        if ((err = flush()) < 0) {
            return err;
        }
        temp = std::min(bytes - offset, buffer_size);
        memcpy(obuff, data + offset, temp);
        buffer_offset = temp;
        offset += temp;
    }

    return err;
}

ssize_t spiFlash::write(const size_t bytes, const char* data) {
    if (!fsMounted || !dataFileOpen) {
        return -1;
    }
    if (data == nullptr) {
        return -2;
    }
    if (bytes == 0) {
        return 0;
    }

    const lfs_ssize_t w = lfs_file_write(&littlefs, &dataFile, data, static_cast<lfs_size_t>(bytes));
    if (w < 0 || static_cast<size_t>(w) != bytes) {
        return -1;
    }
    return static_cast<ssize_t>(w);
}

ssize_t spiFlash::kwrite(const size_t bytes, const char* data) {
    if (!fsMounted || !logFileOpen) {
        return -1;
    }
    if (data == nullptr) {
        return -2;
    }
    if (bytes == 0) {
        return 0;
    }

    const lfs_ssize_t w = lfs_file_write(&littlefs, &logFile, data, static_cast<lfs_size_t>(bytes));
    if (w < 0 || static_cast<size_t>(w) != bytes) {
        return -1;
    }
    return static_cast<ssize_t>(w);
}

int spiFlash::flush(void) {
    if (buffer_offset == 0) {
        return 0;
    }
    ssize_t err = write(buffer_offset, obuff);
    if (err < 0) {
        return -1;
    }

    const int syncErr = lfs_file_sync(&littlefs, &dataFile);
    if (syncErr < 0) {
        return -2;
    }

    memset(obuff, 0, buffer_size);
    buffer_offset = 0;
    return 0;
}

ssize_t spiFlash::kLog(const size_t bytes, const char* data) {
    if (bytes == 0) {
        return 0;
    }
    if (data == nullptr) {
        return -1;
    }
    if (kbuff == nullptr) {
        return -2;
    }

    int err = 0;
    size_t offset = 0;
    size_t temp = 0;

    temp = std::min(bytes, k_buffer_size - k_buffer_offset);
    memcpy(kbuff + k_buffer_offset, data, temp);
    k_buffer_offset += temp;
    offset += temp;

    while (bytes > offset) {
        if ((err = kflush()) < 0) {
            return err;
        }
        temp = std::min(bytes - offset, k_buffer_size - k_buffer_offset);
        memcpy(kbuff + k_buffer_offset, data + offset, temp);
        k_buffer_offset += temp;
        offset += temp;
    }

    return 0;
}

int spiFlash::kflush(void) {
    if (k_buffer_offset == 0) {
        return 0;
    }
    ssize_t err = kwrite(k_buffer_offset, kbuff);
    if (err < 0) {
        return -1;
    }

    const int syncErr = lfs_file_sync(&littlefs, &logFile);
    if (syncErr < 0) {
        return -2;
    }

    memset(kbuff, 0, k_buffer_size);
    k_buffer_offset = 0;
    return 0;
}

ssize_t spiFlash::tick(void) {
    if (queuedos.empty()) {
        return 0;
    }

    ssize_t total = 0;

    // Drain every P_MANDATORY entry currently in the queue (all priority 0),
    // then flush once so mandatory work hits the media together.
    while (!queuedos.empty() && std::get<0>(queuedos.top()) == P_MANDATORY) {
        std::tuple<char, std::vector<char>> item = queuedos.top();
        queuedos.pop();
        const std::vector<char>& payload = std::get<1>(item);
        int e = buffer(payload.size(), payload.data());
        if (e < 0) {
            return static_cast<ssize_t>(e);
        }
        total += static_cast<ssize_t>(payload.size());
    }

    if (total > 0) {
        int fe = flush();
        if (fe < 0) {
            return static_cast<ssize_t>(fe);
        }
        return total;
    }

    // Otherwise process a single highest-priority (non-mandatory top) item.
    std::tuple<char, std::vector<char>> item = queuedos.top();
    queuedos.pop();
    const std::vector<char>& payload = std::get<1>(item);
    int e = buffer(payload.size(), payload.data());
    if (e < 0) {
        return static_cast<ssize_t>(e);
    }
    return static_cast<ssize_t>(payload.size());
}

bool spiFlash::cmp_io_priority::operator()(const std::tuple<char, std::vector<char>>& l,
                                           const std::tuple<char, std::vector<char>>& r) const {
    return std::get<0>(l) > std::get<0>(r);
}

bool spiFlash::exportRootFiles(const SpiFlashExportCallbacks* callbacks) {
    if (callbacks == nullptr || callbacks->onBeginFile == nullptr || callbacks->onWrite == nullptr ||
        callbacks->onEndFile == nullptr) {
        return false;
    }
    if (!fsMounted) {
        return false;
    }

    int fe = flush();
    if (fe < 0) {
        return false;
    }
    fe = kflush();
    if (fe < 0) {
        return false;
    }

    lfs_dir_t root;
    int err = lfs_dir_open(&littlefs, &root, "/");
    if (err < 0) {
        return false;
    }

    lfs_info info;
    while ((err = lfs_dir_read(&littlefs, &root, &info)) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }
        if (info.type != LFS_TYPE_REG) {
            continue;
        }

        lfs_file_t entry;
        err = lfs_file_open(&littlefs, &entry, info.name, LFS_O_RDONLY);
        if (err < 0) {
            lfs_dir_close(&littlefs, &root);
            return false;
        }

        if (!callbacks->onBeginFile(callbacks->user, info.name)) {
            lfs_file_close(&littlefs, &entry);
            lfs_dir_close(&littlefs, &root);
            return false;
        }

        uint8_t buf[512];
        while (true) {
            const lfs_ssize_t n = lfs_file_read(&littlefs, &entry, buf, sizeof(buf));
            if (n < 0) {
                callbacks->onEndFile(callbacks->user);
                lfs_file_close(&littlefs, &entry);
                lfs_dir_close(&littlefs, &root);
                return false;
            }
            if (n == 0) {
                break;
            }
            if (!callbacks->onWrite(callbacks->user, buf, static_cast<size_t>(n))) {
                callbacks->onEndFile(callbacks->user);
                lfs_file_close(&littlefs, &entry);
                lfs_dir_close(&littlefs, &root);
                return false;
            }
        }

        if (!callbacks->onEndFile(callbacks->user)) {
            lfs_file_close(&littlefs, &entry);
            lfs_dir_close(&littlefs, &root);
            return false;
        }

        lfs_file_close(&littlefs, &entry);
    }

    lfs_dir_close(&littlefs, &root);
    return err >= 0;
}

bool spiFlash::removeFile(const char* path) {
    if (path == nullptr || path[0] == '\0' || !fsMounted) {
        return false;
    }
    return lfs_remove(&littlefs, path) == 0;
}

bool spiFlash::mountfs() {
    if (fsMounted) {
        return true;
    }
    if (!lfsConfigured) {
        configureLittleFs();
    }
    const int err = lfs_mount(&littlefs, &lfsConfig);
    if (err < 0) {
        Serial.print("Error, failed to mount LittleFS on SPI flash, error ");
        Serial.println(err);
        fsMounted = false;
        return false;
    }
    fsMounted = true;
    return true;
}

void spiFlash::unmountfs() {
    if (!fsMounted) {
        return;
    }

    flush();
    kflush();
    closeOpenFiles();
    lfs_unmount(&littlefs);
    fsMounted = false;
}

bool spiFlash::isMounted() {
    if (!fsMounted) {
        return false;
    }

    lfs_dir_t root;
    const int err = lfs_dir_open(&littlefs, &root, "/");
    if (err < 0) {
        return false;
    }
    lfs_dir_close(&littlefs, &root);
    return true;
}