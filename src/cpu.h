#ifndef CPU_H
#define CPU_H

#include <cstdint>

// Holds raw CPU tick values read from /proc/stat.
// The Linux kernel exposes cumulative time spent in each CPU state:
//   user, nice, system, idle, iowait, irq, softirq, steal
// We sum everything except idle + iowait to get "active" time.
struct CpuTicks {
    uint64_t user    = 0;
    uint64_t nice    = 0;
    uint64_t system  = 0;
    uint64_t idle    = 0;
    uint64_t iowait  = 0;
    uint64_t irq     = 0;
    uint64_t softirq = 0;
    uint64_t steal   = 0;

    uint64_t total() const;
    uint64_t active() const;
};

// Reads the current CPU tick counters from /proc/stat.
// Returns true on success. On failure (file missing/unreadable), returns false.
bool readCpuTicks(CpuTicks& ticks);

// Calculates CPU usage percentage between two samples.
// The idea: CPU usage = (active_delta / total_delta) * 100
// The first call always returns 0.0 because there is no previous sample to diff against.
//
// Usage pattern:
//   CpuTicks prev, curr;
//   readCpuTicks(prev);
//   // ... sleep 1 second ...
//   readCpuTicks(curr);
//   double usage = cpuUsagePercent(prev, curr);
double cpuUsagePercent(const CpuTicks& prev, const CpuTicks& curr);

#endif
