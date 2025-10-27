#include "sysload.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <sys/statvfs.h>

typedef struct
{
        const char *key;
        size_t key_len;
        uint64_t *field; 
} meminfo_field_t;

static sl_log_handler_t g_log_handler = NULL;
static void *g_log_user_data = NULL;

void sl_set_log_handler(sl_log_handler_t handler, void* user_data)
{
        g_log_handler = handler;
        g_log_user_data = user_data;
}

static void sl_log(sl_log_level_t level, const char* func, const char* fmt, ...)
{
        if (!g_log_handler) return;
        
        char buffer[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        g_log_handler(level, func, buffer, g_log_user_data);       
}


int sleep_float(float seconds)
{
        if (seconds <= 0.0f) return 0;
        struct timespec req, rem;
        req.tv_sec = (time_t)seconds;
        req.tv_nsec = (long)((seconds - req.tv_sec) * 1e9);

        while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
                req = rem;
        }
        return 0; 
}


uint64_t sl_bytes_to_gb(uint64_t bytes)
{
        return bytes / (1024ULL * 1024ULL * 1024ULL);
}

uint64_t sl_kb_to_mb(uint64_t kb)
{
        return kb / 1024ULL;
}


int sl_systime_get_info(sl_systime_info_t *result)
{       
        if (!result) {
                sl_log(SL_LOG_ERROR, __func__, "result pointer is NULL");
                return -1;
        }

        FILE* fptr;
        int ret;

        fptr = fopen("/proc/uptime", "r");
        if (fptr == NULL) {
                sl_log(SL_LOG_ERROR, __func__, "failed to open /proc/uptime");
                return -1;
        }

        ret = fscanf(fptr, "%lf %lf", &result->uptime, &result->idle_time);

        fclose(fptr);

        if (ret == 2) {
                return 0;
        } else if (ret == EOF){
                sl_log(SL_LOG_ERROR, __func__, "failed to read /proc/uptime (EOF)");
        } else {
                sl_log(SL_LOG_ERROR, __func__, "failed to parse /proc/uptime (expected 2 fields, got %d)", ret);
        }

        return -1; 
}


int sl_cpu_get_raw(sl_cpu_raw_t *snapshot)
{
        if (!snapshot) {
                sl_log(SL_LOG_ERROR, __func__, "snapshot pointer is NULL");
                return -1;
        }

        FILE* fptr;
        int ret;

        fptr = fopen("/proc/stat", "r");
        if (fptr == NULL) {
                sl_log(SL_LOG_ERROR, __func__, "failed to open /proc/stat");
                return -1;
        }

        ret = fscanf(fptr, "%*s" " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                     &snapshot->user,
                     &snapshot->nice,
                     &snapshot->system,
                     &snapshot->idle,
                     &snapshot->iowait,
                     &snapshot->irq,
                     &snapshot->softirq,        
                     &snapshot->steal);
        fclose(fptr); 

        if (ret == 8) {
                return 0;
        } else if (ret == EOF) {
                sl_log(SL_LOG_ERROR, __func__, "failed to read from /proc/stat (EOF)");
        } else {
                sl_log(SL_LOG_ERROR, __func__, "failed to parse /proc/stat (expected 8 fields, got %d)", ret);
        }
        return -1;
}


int sl_cpu_calculate(const sl_cpu_raw_t *start, const sl_cpu_raw_t *end, sl_cpu_usage_t *result)
{       
        if (!start || !end || !result) {
                sl_log(SL_LOG_ERROR, __func__, "start, end, result pointers are NULL");
                return -1;
        }

        uint64_t total_start, total_end;
        uint64_t total_diff;

        total_start = start->user
                    + start->nice
                    + start->system
                    + start->idle
                    + start->iowait
                    + start->irq
                    + start->softirq
                    + start->steal;
        
        total_end = end->user
                  + end->nice
                  + end->system
                  + end->idle
                  + end->iowait
                  + end->irq
                  + end->softirq
                  + end->steal;

        if (total_end <= total_start) {
                sl_log(SL_LOG_ERROR, __func__, "invalid time interval or counter overflow");
                return -1;
        }

        total_diff = total_end - total_start;

        if (total_diff == 0) {
                memset(result, 0, sizeof(sl_cpu_usage_t));
                return 0;
        }
 
        result->user    = ((float)(end->user - start->user) / total_diff) * 100.0f;
        result->nice    = ((float)(end->nice - start->nice) / total_diff) * 100.0f;
        result->system  = ((float)(end->system - start->system) / total_diff) * 100.0f;
        result->idle    = ((float)(end->idle - start->idle) / total_diff) * 100.0f;
        result->iowait  = ((float)(end->iowait - start->iowait) / total_diff) * 100.0f;
        result->irq     = ((float)(end->irq - start->irq) / total_diff) * 100.0f;
        result->softirq = ((float)(end->softirq - start->softirq) / total_diff) * 100.0f;
        result->steal   = ((float)(end->steal - start->steal) / total_diff) * 100.0f;
        
        result->total   = 100.0f - (result->idle + result->iowait);

        return 0;
}


int sl_cpu_get_usage(float interval_sec, sl_cpu_usage_t *result)
{       
        if (!result) {
                sl_log(SL_LOG_ERROR, __func__, "result pointer is NULL");
                return -1;
        }

        if (interval_sec < 0.1f) {
                sl_log(SL_LOG_ERROR, __func__, "interval too small, minimum is 0.1 seconds");
                return -1;
        }

        sl_cpu_raw_t start, end;

        if (sl_cpu_get_raw(&start)) {
                sl_log(SL_LOG_ERROR, __func__, "failed to get CPU start snapshot");
                return -1;
        }

        if (sleep_float(interval_sec) == -1) {
                sl_log(SL_LOG_ERROR, __func__, "sleep failed");
                return -1;
        }

        if (sl_cpu_get_raw(&end)) {
                sl_log(SL_LOG_ERROR, __func__, "failed to get CPU end snapshot");
                return -1;
        }

        return sl_cpu_calculate(&start, &end, result);
}


int parse_meminfo_line(const char *line, sl_mem_info_t *result)
{       
        char key[MEMINFO_KEY_SIZE];
        uint64_t value = 0;

        if (sscanf(line, "%31[^:]: %" SCNu64, key, &value) != 2) {
                sl_log(SL_LOG_ERROR, __func__, "failed to parse meminfo line: %.100s", line);
                return -1;
        }
 
        meminfo_field_t fields[] = {
                {"MemTotal",     8,  &result->total},
                {"MemFree",      7,  &result->free},
                {"MemAvailable", 12, &result->available},
                {"Buffers",      7,  &result->buffers},
                {"Cached",       6,  &result->cached},
                {"Shmem",        5,  &result->shared}, 
                {"SwapTotal",    9,  &result->swap_total},
                {"SwapFree",     8,  &result->swap_free},
                {NULL,           0,  NULL}
        };

        for (size_t i = 0; fields[i].key != NULL; i++) {
                if (strncmp(key, fields[i].key, fields[i].key_len) == 0 &&
                            key[fields[i].key_len] == '\0') {
                        *(fields[i].field) = value;
                        return 0;
                }
        }

        return -1;
}


int sl_mem_calculate(sl_mem_info_t *result) 
{       
        if (!result) {
                sl_log(SL_LOG_ERROR, __func__, "result pointer is NULL");
                return -1;
        }

        if (result->total == 0) {
                sl_log(SL_LOG_ERROR, __func__, "total memory is zero");
                return -1;
        }

        if (result->available > 0) {
                result->used = result-> total - result->available;
        } else {
                sl_log(SL_LOG_WARN, __func__, "available memory is zero, falling back to total - free - buffers - cached");
                result->used = result->total - result->free - result->buffers - result->cached;
                if (result->used > result->total) {
                        result->used = result->total;
                }
        }
        result->percent_used = ((float)result->used / result->total) * 100.0f;

        result->swap_used = (result->swap_total > 0) ? (result->swap_total - result->swap_free) : 0;

        return 0;
}


int sl_mem_get_info(sl_mem_info_t *result)
{       
        if (!result) {
                sl_log(SL_LOG_ERROR, __func__, "result pointer is NULL");
                return -1;
        }

        FILE* fptr;

        fptr = fopen("/proc/meminfo", "r");
        if (fptr == NULL) {
                sl_log(SL_LOG_ERROR, __func__, "failed to open /proc/meminfo");
                fclose(fptr);
                return -1;
        }

        memset(result, 0, sizeof(sl_mem_info_t));

        char line[MEMINFO_LINE_SIZE];
        int missing_fields = EXPECTED_MEMINFO_KEYS; 

        while(fgets(line, sizeof(line), fptr) != NULL) {
                parse_meminfo_line(line, result);
        }

        fclose(fptr);


        if (sl_mem_calculate(result) != 0) {
                sl_log(SL_LOG_ERROR, __func__, "failed to calculate memory usage");
                return -1;
        }

        return 0;
}


int sl_storage_get_info(const char *path, sl_storage_info_t *result)
{       
        if (!result) {
                sl_log(SL_LOG_ERROR, __func__, "result pointer is NULL");
                return -1;
        }

        struct statvfs svfs;

        if (statvfs(path, &svfs) == -1) {
                sl_log(SL_LOG_ERROR, __func__, "failed to get filesystem info for %s", path);
                return -1;
        }

        uint64_t block_size = svfs.f_frsize;

        result->total = (uint64_t)svfs.f_blocks * block_size;
        result->free = (uint64_t)svfs.f_bfree * block_size;
        result->available = (uint64_t)svfs.f_bavail * block_size;
        
        result->used = result->total - result->free;

        if (result->total == 0) {
                result->percent_usage = 0.0;
        } else {
        result->percent_usage = ((double)result->used / result->total) * 100.0;
        }
        
        return 0; 
}
