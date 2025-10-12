#ifndef SYSLOAD_H
#define SYSLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/* System time information */
typedef struct {
        double uptime;
        double idle_time;
} sl_systime_info_t;

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

/* Memory information (all values in kilobytes) */
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

/* Filesysten storage information */
typedef struct
{
        uint64_t total;
        uint64_t free;
        uint64_t available;
        uint64_t used;
        double percent_usage;
} sl_storage_info_t;

/**
 * @brief Get systime information from /proc/uptime
 * @param result Pointer to store raw values
 * @return 0 on success, -1 on error
 */
int sl_systime_get_info(sl_systime_info_t *result);

/**
 * @brief Get raw CPU counters from /proc/stat
 * @param snapshot Pointer to store raw counters
 * @return 0 on success, -1 on error 
 */
int sl_cpu_get_raw(sl_cpu_raw_t *snapshot);

/**
 * @brief Calculate CPU usage between two snapshots
 * @param start First snapshot
 * @param end Second snapshot
 * @param result Pointer to store calculated percentages
 * @return 0 on success, -1 on error
 */
int sl_cpu_calculate(const sl_cpu_raw_t *start, const sl_cpu_raw_t *end, sl_cpu_usage_t *result);

/**
 * @brief Get CPU usage over a time interval
 * @param interval_sec Measurement interval in seconds 
 * @param result Pointer to store CPU usage percentages
 * @return 0 on success, -1 on error
 */
int sl_cpu_get_usage(float interval_sec, sl_cpu_usage_t *result);


/**
 * @brief Calculate derived memory statistics
 * @param result Memory into structure with raw values
 * @return 0 on success, -1 if calculation failed
 */
int sl_mem_calculate(sl_mem_info_t *result);

/**
 * @brief Get memory information from /proc/meminfo
 * @param result Pointer to store memory information
 * @return Number of successfully expected parsed fields, or -1 on error
 */
int sl_mem_get_info(sl_mem_info_t *result);

/**
 * @brief Get filesystem storage information for mounted path
 * @param path Filesystem path to check
 * @param result Pointer to store storage information
 * @return 0 on success, -1 on error
 */
int sl_storage_get_info(const char *path, sl_storage_info_t *result);

#ifdef __cplusplus
}
#endif

#endif