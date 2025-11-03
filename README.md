# sysload - Lightweight Linux system resource monitor (C library)

'sysload' is a small, dependency-free C library that provides access to basic Linux system metrics - CPU usage, memory and swap stats, storage usage, and uptime - by parsing '/proc' and system calls.

Perfect for lightweight tools, embedded systems, or as an educational example of '/proc' parsing.

---

## ✨ Features
- CPU usage calculation via '/proc/stat'
- Memory and swap information from '/proc/meminfo'
- Filesystem statistics via 'statvfs()'
- System uptime and idle time from '/proc/uptime'
- Simple API, no dependencies
- Works on any modern Linux system
- Optional logging via user-provided callback

---

## ⚙️ Build and Install
Requirements
- C compiler
- CMake ≥ 3.10

### Build
```bash
git clone https://github.com/ugolnikovE/sysload.git
cd sysload
cmake -B build
cmake --build build
```

### Install system-wide
```bash
sudo cmake --install build
```
Libraries will be installed to `/usr/local/lib` and headers to `/usr/local/include/sysload`

---

## 🔗 Use in another CMake project
If you want to embed the library directly into your project (no system install required), you can use `FetchContent`:
```cmake
include(FetchContent)

FetchContent_Declare(
        sysload
        GIT_REPOSITORY https://github.com/ugolnikovE/sysload.git
        GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(sysload)

target_link_libraries(myapp PRIVATE sysload)
```

---

## 💻 Example usage
Below is a minimal example showing how to use sysload in your own project after installing the library.
```C
#include <sysload.h>
#include <stdio.h>
#include <inttypes.h>

int main(void)
{
        sl_cpu_usage_t cpu;
        if (sl_cpu_get_usage(1.0f, &cpu) == 0)
                printf("CPU total usage: %.2f%%\n", cpu.total);

        sl_mem_info_t mem;
        if (sl_mem_get_info(&mem) == 0)
        printf("Memory used: %.2f%%\n", mem.percent_used);

        sl_storage_info_t disk;
        if (sl_storage_get_info("/", &disk) == 0)
                printf("Disk used: %.2f%%\n", disk.percent_usage);

        sl_systime_info_t sys;
        if (sl_systime_get_info(&sys) == 0)
                printf("Uptime: %.0lf seconds\n", sys.uptime);

        return 0;
}
```
Compile and Run:
```bash
gcc main.c -o main -lsysload
./main
```

## ❌ Uninstall
To delete the library from the system, use the command:
```
sudo cmake --build build --target uninstall
```


## 📁 Project structure
```
include/ -> public headers
src/     -> Library source code
example/ -> Example usage program
tests/   -> Simple tests for library
build/   -> Build artifacts
CMakeLists.txt
```

## 📚 License
Licensed under the [MIT License](LICENSE) — free for personal and commercial use.

## 💡 Motivation
This library was written as a learning project to explore Linux /proc parsing in C.
It aims to be simple, well-documented, and practical enough for small tools or embedded monitoring.

## 🤝 Contributions
Pull requests and feedback are welcome!
If you use sysload in your project, feel free to share it — it helps improve the library.