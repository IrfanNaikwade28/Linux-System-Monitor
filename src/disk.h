#ifndef DISK_H
#define DISK_H

#include <cstdint>
#include <string>

struct DiskInfo {
    uint64_t totalBytes     = 0;
    uint64_t usedBytes      = 0;
    uint64_t availableBytes = 0;
    double   usedPercent    = 0.0;
};

// Uses the POSIX statvfs() system call to query filesystem statistics.
// statvfs returns block size, total blocks, and free blocks for a given path.
//
// We query "/" (root filesystem) by default.
// Returns true on success, false if statvfs fails.
bool readDiskInfo(DiskInfo& info, const std::string& path = "/");

#endif
