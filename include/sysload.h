#ifndef SYSLOAD_H
#define SYSLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Raw CPU counters snapshot */
typedef struct
{
        uint64_t user;
        uint64_t nice;
        uint64_t system;
        uint64_t idle;
        uint64_t iowait;
        uint64_t irq;
        uint64_t softirq;
        uint64_t steal; 
} sl_cpu_raw_t;

/* Calculated CPU usage percentages */
typedef struct
{
        float user;
        float nice;
        float system;
        float idle;
        float iowait;
        float irq;
        float softirq;
        float steal;
        float total;
} sl_cpu_usage_t;

/* Memory information */
typedef struct
{
        uint64_t total;
        uint64_t free;
        uint64_t available;
        uint64_t buffers;
        uint64_t cached; 
        uint64_t shared;
        uint64_t used;
        uint64_t swap_total;
        uint64_t swap_free;
        uint64_t swap_used;
        float percent_used;
} sl_mem_info_t;

int sl_cpu_get_raw(sl_cpu_raw_t *snapshot);
int sl_cpu_calculate(const sl_cpu_raw_t *start, const sl_cpu_raw_t *end, sl_cpu_usage_t *result);
int sl_cpu_get_usage(float interval_sec, sl_cpu_usage_t *result);


#ifdef __cplusplus
}
#endif

#endif