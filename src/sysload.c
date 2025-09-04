#include "sysload.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>

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


int sl_cpu_get_raw(sl_cpu_raw_t *snapshot)
{
        FILE* fptr;
        int ret;

        fptr = fopen("/proc/stat", "r");
        if (fptr == NULL) {
                perror("Can't open /proc/stat file");
                return -1;
        }

        ret = fscanf(fptr, "%*s %llu %llu %llu %llu %llu %llu %llu %llu",
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
        } else if (ret = EOF) {
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
