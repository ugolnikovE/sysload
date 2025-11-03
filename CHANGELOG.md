## 0.1.1 (2025-11-03)
### Fixes:
- CMake build and FetchContent support 

## 0.1.0 (2025-11-02)
Initial public release.

### Features:
- CPU usage calculation via '/proc/stat'
- Memory and swap information from '/proc/meminfo'
- Filesystem statistics via 'statvfs()'
- System uptime and idle time from '/proc/uptime'
- Simple API, no dependencies
- Works on any modern Linux system
- Optional logging via user-provided callback

