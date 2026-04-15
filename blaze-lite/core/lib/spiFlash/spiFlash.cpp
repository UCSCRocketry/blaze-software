/*
    * spiFlash.cpp - SPI Flash library for Blaze
    * To use:
        include the corresponding header file in your main program, DO NOT directly call this cpp file
*/
#include "spiFlash.h"

#include <SPI.h>
#include <SdFat.h>

#include <Adafruit_SPIFlash.h>
#include <flash_devices.h>

#include <algorithm>

namespace {

const uint8_t kFlashCsPin = PB8;

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

FatVolume fatfs;

File32 dataFile;
File32 logFile;

void makeDataFileName(char* buffer, size_t bufferSize) {
    uint16_t fileIndex = 0;
    do {
        snprintf(buffer, bufferSize, "DATA%03u.txt", fileIndex++);
    } while (fatfs.exists(buffer) && fileIndex < 1000);
}

void makeLogFileName(char* buffer, size_t bufferSize) {
    uint16_t fileIndex = 0;
    do {
        snprintf(buffer, bufferSize, "LOG%03u.txt", fileIndex++);
    } while (fatfs.exists(buffer) && fileIndex < 1000);
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
    kflush();
    flush();
    logFile.close();
    dataFile.close();
    delete[] obuff;
    delete[] kbuff;
}

// https://github.com/adafruit/Adafruit_SPIFlash/blob/master/examples/SdFat_datalogging/SdFat_datalogging.ino
bool spiFlash::startUp() {
    // Pass explicit candidates: W25Q128JV-PM/IM (EF 70 18) is omitted from Adafruit's built-in list,
    // which only includes W25Q128JV-SQ (EF 40 18).
    if (!flashChip.begin(kBlazeFlashCandidates,
                         sizeof(kBlazeFlashCandidates) / sizeof(kBlazeFlashCandidates[0]))) {
        Serial.println("Error, failed to initialize flash chip!");
        logRawJedec(kFlashCsPin);
        return false;
    }
    Serial.print("Flash chip JEDEC ID: 0x");
    Serial.println(flashChip.getJEDECID(), HEX);

    if (!fatfs.begin(&flashChip)) {
        Serial.println("Error, failed to mount filesystem on SPI flash!");
        Serial.println("Attempting to format SPI flash as FAT...");

        dataFile.close();
        logFile.close();

        uint8_t formatWorkBuf[512];
        FatFormatter formatter;
        if (!formatter.format(&flashChip, formatWorkBuf, &Serial)) {
            Serial.println("Error, failed to format SPI flash filesystem!");
            return false;
        }

        if (!fatfs.begin(&flashChip)) {
            Serial.println("Error, format completed but remount failed!");
            return false;
        }
        Serial.println("Formatted and mounted SPI flash filesystem.");
    } else {
        Serial.println("Mounted SPI flash filesystem.");
    }

    char dataFileName[16];
    char logFileName[16];
    makeDataFileName(dataFileName, sizeof(dataFileName));
    makeLogFileName(logFileName, sizeof(logFileName));

    dataFile.close();
    logFile.close();

    dataFile = fatfs.open(dataFileName, FILE_WRITE);
    logFile = fatfs.open(logFileName, FILE_WRITE);
    delay(500);

    fatfs.ls(Serial);

    if (!dataFile) {
        Serial.println("Failed to open SPI flash data file");
        return false;
    }
    Serial.print("SPI flash data file: ");
    Serial.println(dataFileName);

    if (!logFile) {
        Serial.println("Failed to open SPI flash log file");
        return false;
    }
    Serial.print("SPI flash log file: ");
    Serial.println(logFileName);

    dataFile.close();
    logFile.close();

    Serial.println("file closed");

    //unmount fs
    // TODO Need to unmount fs system during fatfs.end();

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
    if (!dataFile) {
        return -2;
    }
    if (!dataFile.seekSet(static_cast<uint32_t>(offset))) {
        return -3;
    }
    int n = dataFile.read(buffer, bytes);
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
    if (!dataFile) {
        return -1;
    }
    if (bytes == 0) {
        return 0;
    }
    size_t w = dataFile.write(data, bytes);
    if (w != bytes) {
        return -1;
    }
    return static_cast<ssize_t>(w);
}

ssize_t spiFlash::kwrite(const size_t bytes, const char* data) {
    if (!logFile) {
        return -1;
    }
    if (bytes == 0) {
        return 0;
    }
    size_t w = logFile.write(data, bytes);
    if (w != bytes) {
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

    int fe = flush();
    if (fe < 0) {
        return false;
    }
    fe = kflush();
    if (fe < 0) {
        return false;
    }

    FatFile root;
    if (!root.openRoot(&fatfs)) {
        return false;
    }

    FatFile entry;
    while (entry.openNext(&root, O_RDONLY)) {
        if (!entry.isFile()) {
            entry.close();
            continue;
        }

        char name[64];
        if (entry.getName(name, sizeof(name)) == 0) {
            entry.close();
            continue;
        }

        if (!callbacks->onBeginFile(callbacks->user, name)) {
            entry.close();
            root.close();
            return false;
        }

        uint8_t buf[512];
        int n;
        while (true) {
            n = entry.read(buf, sizeof(buf));
            if (n < 0) {
                callbacks->onEndFile(callbacks->user);
                entry.close();
                root.close();
                return false;
            }
            if (n == 0) {
                break;
            }
            if (!callbacks->onWrite(callbacks->user, buf, static_cast<size_t>(n))) {
                callbacks->onEndFile(callbacks->user);
                entry.close();
                root.close();
                return false;
            }
        }

        if (!callbacks->onEndFile(callbacks->user)) {
            entry.close();
            root.close();
            return false;
        }

        entry.close();
    }

    root.close();
    return true;
}

bool spiFlash::removeFile(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return false;
    }
    return fatfs.remove(path);
}
