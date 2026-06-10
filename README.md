# Linux System Monitor

A simple, practical C++ terminal-based system monitor for Linux. It reads live system data from the `/proc` filesystem and uses POSIX APIs to display CPU, memory, disk, uptime, and process information. Refreshes every 1 second.

Built as a systems-programming learning project that demonstrates Linux/POSIX APIs, `/proc` filesystem usage, and real-time terminal output -- no external dependencies, no GUI, no third-party libraries.

## Features

- **CPU usage** -- calculates real CPU usage percentage from `/proc/stat` tick counters
- **Memory usage** -- reads total, available, and used memory from `/proc/meminfo`
- **Disk usage** -- uses the POSIX `statvfs()` system call on the root filesystem
- **System uptime** -- reads from `/proc/uptime`
- **Process count** -- counts all `/proc/[pid]` entries
- **Top processes** -- scans `/proc/[pid]/comm` and `/proc/[pid]/statm` to show processes sorted by memory usage
- **1-second refresh** -- clears and redraws the terminal every second

## Technologies

- **Language:** C++17
- **Compiler:** g++
- **Platform:** Linux only
- **APIs used:**
  - `/proc/stat` -- CPU statistics
  - `/proc/meminfo` -- memory information
  - `/proc/uptime` -- system uptime
  - `/proc/[pid]/comm` -- process names
  - `/proc/[pid]/statm` -- process memory usage
  - `statvfs()` -- POSIX filesystem statistics
  - `opendir()`/`readdir()` -- POSIX directory traversal
  - `sleep()` -- POSIX timing

No external libraries. No Boost. No ncurses. No GUI.

## Project Structure

```
.
├── Makefile
├── README.md
├── .gitignore
└── src/
    ├── main.cpp       # Terminal display and refresh loop
    ├── cpu.h          # CPU usage interface
    ├── cpu.cpp        # Reads /proc/stat, computes usage percentage
    ├── memory.h       # Memory usage interface
    ├── memory.cpp     # Reads /proc/meminfo, formats byte sizes
    ├── process.h      # Process scanning interface
    ├── process.cpp    # Scans /proc/[pid], reads comm and statm
    ├── disk.h         # Disk usage interface
    ├── disk.cpp       # Uses POSIX statvfs() for filesystem stats
    ├── system.h       # Uptime interface
    └── system.cpp     # Reads /proc/uptime, formats duration
```

## Build

```bash
make
```

## Run

```bash
make run
```

Or directly:

```bash
./sysmon
```

## Clean

```bash
make clean
```

## Example Output

```
Linux System Monitor
----------------------------
CPU Usage      : 3.2%
Memory Usage   : 4.8 GB / 7.6 GB (63%)
Disk Usage     : 78.2 GB / 120.0 GB (65%)
System Uptime  : 2h 34m
Processes      : 184
----------------------------
Top Processes (by memory)
PID      NAME                 MEMORY
1234     firefox              1.2 GB
5678     code                 845 MB
9012     gnome-shell          312 MB
3456     Xorg                 128 MB
7890     bash                 4 MB
...
```

## How It Works

### What is /proc?

`/proc` is a virtual filesystem in Linux (called a "procfs"). It is not stored on disk -- the kernel generates its contents on-the-fly when you read from it. It exposes kernel and process data as plain text files, which makes it easy to read from any language.

Key files we use:
| File | Content |
|---|---|
| `/proc/stat` | Aggregate CPU time in each state (user, system, idle, etc.) since boot |
| `/proc/meminfo` | Memory statistics (total, free, available, cached, etc.) |
| `/proc/uptime` | System uptime in seconds |
| `/proc/[pid]/comm` | Process command name (executable name, max 15 chars) |
| `/proc/[pid]/statm` | Process memory usage in pages |

### CPU Usage Calculation

CPU usage is calculated by taking two snapshots of `/proc/stat` and computing the difference:

```
Snapshot 1: total=1000, active=200  (idle=800)
Snapshot 2: total=1100, active=280  (idle=820)

active_delta = 280 - 200 = 80
total_delta  = 1100 - 1000 = 100

CPU usage = (80 / 100) * 100 = 80.0%
```

This is the same method used by tools like `top`, `htop`, and `vmstat`. The values in `/proc/stat` are cumulative jiffies (typically 1/100th of a second). Taking the difference between two readings gives the proportion of time the CPU spent doing work vs. idling.

### Memory Calculation

We read `MemTotal` and `MemAvailable` from `/proc/meminfo`:

```
Used = MemTotal - MemAvailable
Percent = (Used / MemTotal) * 100
```

`MemAvailable` is an estimate by the kernel of memory available for new applications, accounting for page cache and buffers that can be reclaimed. This matches how the `free` command works.

### Process Scanning

We traverse `/proc/` looking for numeric directory names (PIDs). For each PID, we read:
- `/proc/[pid]/comm` -- the process name
- `/proc/[pid]/statm` -- resident set size in pages

We then convert pages to kilobytes (page size * 4 KB) and sort by memory usage.

**Why processes can disappear during scanning:** Between listing the directory and reading a PID's files, the process may exit. The kernel removes the `/proc/[pid]` directory immediately. We handle this by catching file-open failures and skipping those PIDs silently.

### Disk Usage

We use the POSIX `statvfs()` system call, which returns filesystem block statistics:

```
total     = f_blocks * f_frsize
available = f_bavail * f_frsize
used      = total - available
percent   = (used / total) * 100
```

`f_bavail` (not `f_bfree`) is used because it represents space available to non-root users, which matches what `df` shows.

### Refresh Loop

The main loop:
1. Read all system data (CPU, memory, disk, uptime, processes)
2. Clear the terminal using ANSI escape codes (`\033[2J\033[H`)
3. Print all data
4. Sleep for 1 second using `sleep(1)`
5. Repeat

This is intentionally simple -- no threading, no ncurses, no complex UI. Just stdout + ANSI escape codes.

## Limitations

- **Single-threaded refresh:** The sleep blocks everything. On a loaded system, process scanning may take a few milliseconds, slightly affecting refresh timing.
- **Process count only:** We count all processes but don't distinguish between threads, zombies, or kernel threads.
- **No historical data:** Only shows the current moment -- no graphs, no trending, no logging.
- **Root filesystem only:** Disk monitoring only covers `/`. Could be extended to show all mounted filesystems.
- **No per-CPU stats:** Only shows aggregate CPU usage. Could be extended to show individual cores.

## Possible Future Improvements

- Per-CPU core usage display
- Network interface monitoring (`/proc/net/dev`)
- Temperature reading (`/sys/class/thermal/`)
- Load average display (`/proc/loadavg`)
- Process filtering by user or name
- Configurable refresh interval
- All mounted filesystems disk usage

## License

This project is provided as-is for educational purposes.
