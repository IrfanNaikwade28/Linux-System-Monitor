#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <string>

struct MemoryInfo {
    uint64_t totalKb     = 0;
    uint64_t availableKb = 0;
    uint64_t usedKb      = 0;
    double   usedPercent = 0.0;
};

// Reads /proc/meminfo and populates MemoryInfo.
// Returns true on success, false if the file cannot be read.
// The kernel exposes "MemTotal" and "MemAvailable" in /proc/meminfo.
// We compute used = total - available (this matches what `free` reports).
bool readMemoryInfo(MemoryInfo& info);

// Formats kilobytes into a human-readable string like "4.8 GB" or "512 MB".
std::string formatBytes(uint64_t kb);

#endif
