#include "sysload.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/statvfs.h>


#define MEMINFO_LINE_SIZE 256
#define MEMINFO_KEY_SIZE 32
#define EXPECTED_MEMINFO_KEYS 8


typedef struct
{
        const char *key;
        size_t key_len;
        uint64_t *field; 
} meminfo_field_t;


int sleep_float(float seconds)
{
        struct timespec req;
        float sec_frac;

        if (seconds <= 0.0f) {
                return 0;
        }

        req.tv_sec = (time_t)seconds;
        sec_frac = seconds - (float)req.tv_sec;
        req.tv_nsec = (long)(sec_frac * 1000000000L);

        while (nanosleep(&req, &req) == -1) {
                if (errno != EINTR) {
                        return -1;
                }
        }
        return 0;
}


int sl_systime_get_info(sl_systime_info_t *result)
{
        FILE* fptr;
        int ret;

        fptr = fopen("/proc/uptime", "r");
        if (fptr == NULL) {
                perror("Can't open /proc/stat file");
                return -1;
        }

        ret = fscanf(fptr, "%lf %lf", &result->uptime, &result->idle_time);

        fclose(fptr);

        if (ret == 2) {
                return 0;
        } else if (ret == EOF){
                perror("Failed to read from /proc/uptime (EOF)");
        } else {
                fprintf(stderr, "Failed to parse /proc/uptime"); 
        }

        return -1; 
}


int sl_cpu_get_raw(sl_cpu_raw_t *snapshot)
{
        FILE* fptr;
        int ret;

        fptr = fopen("/proc/stat", "r");
        if (fptr == NULL) {
                perror("Can't open /proc/stat file");
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
                perror("Failed to read from /proc/stat (EOF)");
        } else {
                fprintf(stderr, "Failed to parse /proc/stat: expected 8 fields, got %d\n", 
                        ret); 
        }
        return -1;
}


int sl_cpu_calculate(const sl_cpu_raw_t *start, const sl_cpu_raw_t *end, sl_cpu_usage_t *result)
{
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
                fprintf(stderr, "Error: invalid time interval or counter overflow\n");
                return -1;
        }

        total_diff = total_end - total_start;

        if (total_diff == 0) {
                fprintf(stderr, "Error: zero time interval between measurements\n");
                return -1;
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
        sl_cpu_raw_t start, end;

        if (sl_cpu_get_raw(&start)) {
                fprintf(stderr, "Can't get CPU start snapshot\n");
                return -1;
        }

        if (sleep_float(interval_sec) == -1) {
                perror("Sleep failed");
                return -1;
        }

        if (sl_cpu_get_raw(&end)) {
                fprintf(stderr, "Can't get CPU end snapshot\n");
                return -1;
        }

        return sl_cpu_calculate(&start, &end, result);
}


int parse_meminfo_line(const char *line, sl_mem_info_t *result)
{       
        char key[MEMINFO_KEY_SIZE];
        uint64_t value = 0;

        if (sscanf(line, "%31[^:]: %" SCNu64, key, &value) != 2) {
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
        int ret = 0; 
        if ((result->total > 0) && (result->available > 0)) {
                result->used = result->total - result->available;
                result->percent_used = ((float)(result->used) / result->total) * 100.0f;
        } else {
                ret = -1;
        }

        if (result->swap_total > 0 && result->swap_free > 0) {
                result->swap_used = result->swap_total - result->swap_free;
        }

        return ret;
}


int sl_mem_get_info(sl_mem_info_t *result)
{
        FILE* fptr;

        fptr = fopen("/proc/meminfo", "r");
        if (fptr == NULL) {
                perror("Can't open /proc/meminfo file");
                return -1;
        }

        memset(result, 0, sizeof(sl_mem_info_t));

        char line[MEMINFO_LINE_SIZE];
        int missing_fields = EXPECTED_MEMINFO_KEYS; 

        while(fgets(line, sizeof(line), fptr) != NULL) {
                if (parse_meminfo_line(line, result) == 0) {
                        missing_fields--;
                }
                
        }

        if (sl_mem_calculate(result)) {
                fprintf(stderr, "Failed calculate meminfo usage\n");
        }

        fclose(fptr);
        return (EXPECTED_MEMINFO_KEYS - missing_fields);
}


int sl_storage_get_info(const char *path, sl_storage_info_t *result)
{
        struct statvfs svfs;

        if (statvfs(path, &svfs) == -1) {
                perror("statvfs");
                return -1;
        }

        uint64_t block_size = svfs.f_frsize;

        result->total = (uint64_t)svfs.f_blocks * block_size;
        result->free = (uint64_t)svfs.f_bfree * block_size;
        result->available = (uint64_t)svfs.f_bavail * block_size;
        
        result->used = result->total - result->free;

        result->percent_usage = ((double)result->used / result->total) * 100.0;

        return 0; 
}
