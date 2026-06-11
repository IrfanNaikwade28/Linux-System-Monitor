#include "disk.h"
#include <sys/statvfs.h>
#include <cstdio>

/*
 * statvfs() is a POSIX system call that returns filesystem statistics:
 *
 *   struct statvfs {
 *       unsigned long f_bsize;   // Filesystem block size
 *       unsigned long f_frsize;  // Fragment size (often same as f_bsize)
 *       fsblkcnt_t    f_blocks;  // Total blocks
 *       fsblkcnt_t    f_bfree;   // Free blocks (including reserved)
 *       fsblkcnt_t    f_bavail;  // Free blocks available to non-root users
 *       ...
 *   };
 *
 * To compute disk usage:
 *   total     = f_blocks * f_frsize
 *   available = f_bavail * f_frsize
 *   used      = total - available
 *   percent   = (used / total) * 100
 *
 * We use f_bavail (not f_bfree) because f_bfree includes blocks reserved
 * for the superuser. On most Linux systems, root reserves ~5% of disk
 * space so the system doesn't run completely out of room. `df` uses
 * f_bavail as well, so our output matches `df` behavior.
 */

bool readDiskInfo(DiskInfo& info, const std::string& path) {
    struct statvfs stats;

    // statvfs() is a POSIX C function -- a direct system call wrapper.
    if (statvfs(path.c_str(), &stats) != 0) {
        return false;
    }

    uint64_t blockSize   = stats.f_frsize;
    uint64_t totalBlocks = stats.f_blocks;
    uint64_t freeBlocks  = stats.f_bavail;  // available to non-root

    info.totalBytes     = totalBlocks * blockSize;
    info.availableBytes = freeBlocks  * blockSize;
    info.usedBytes      = info.totalBytes - info.availableBytes;

    if (info.totalBytes == 0) {
        info.usedPercent = 0.0;
    } else {
        info.usedPercent = (static_cast<double>(info.usedBytes) /
                            static_cast<double>(info.totalBytes)) * 100.0;
    }

    return true;
}
