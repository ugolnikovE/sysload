#ifndef SYSLOAD_H
#define SYSLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MEMINFO_LINE_SIZE 256
#define MEMINFO_KEY_SIZE 32
#define EXPECTED_MEMINFO_KEYS 8

#define SYSLOAD_VERSION_MAJOR 0
#define SYSLOAD_VERSION_MINOR 1
#define SYSLOAD_VERSION_PATCH 1
#define SYSLOAD_VERSION "0.1.1"


/* ============================================================= */
/*                        UNIT DEFINITIONS                       */
/* ============================================================= */

/* Memory values are in KB */
typedef struct
{
    uint64_t total;        /**< Total memory in KB */
    uint64_t free;         /**< Free memory in KB */
    uint64_t available;    /**< Available memory in KB */
    uint64_t buffers;      /**< Buffers memory in KB */
    uint64_t cached;       /**< Cached memory in KB */
    uint64_t shared;       /**< Shared memory in KB */
    uint64_t used;         /**< Used memory in KB (calculated) */
    uint64_t swap_total;   /**< Total swap memory in KB */
    uint64_t swap_free;    /**< Free swap memory in KB */
    uint64_t swap_used;    /**< Used swap memory in KB */
    float percent_used;    /**< Memory usage percentage */
} sl_mem_info_t;

/* CPU usage percentages */
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
    float total;           /**< Total CPU usage (100 - idle - iowait) */
} sl_cpu_usage_t;

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

/* System time information (seconds) */
typedef struct
{
    double uptime;          /**< System uptime in seconds */
    double idle_time;       /**< Total idle time in seconds */
} sl_systime_info_t;

/* Filesystem storage information (values in bytes) */
typedef struct
{
    uint64_t total;         /**< Total storage in bytes */
    uint64_t free;          /**< Free storage in bytes */
    uint64_t available;     /**< Available storage in bytes */
    uint64_t used;          /**< Used storage in bytes */
    double percent_usage;   /**< Percentage of used storage */
} sl_storage_info_t;

/* ============================================================= */
/*                        FUNCTION PROTOTYPES                     */
/* ============================================================= */

/* ------------------- Conversion helpers ---------------------- */

/**
 * @brief Convert bytes to GB (rounded down)
 * @param bytes Input value in bytes
 * @return Value in GB
 */
uint64_t sl_bytes_to_gb(uint64_t bytes);

/**
 * @brief Convert KB to MB (rounded down)
 * @param kb Input value in KB
 * @return Value in MB
 */
uint64_t sl_kb_to_mb(uint64_t kb);

/* ------------------- System time ----------------------------- */

/**
 * @brief Get system uptime and idle time from /proc/uptime
 * @param result Pointer to store values
 * @return 0 on success, -1 on error
 */
int sl_systime_get_info(sl_systime_info_t *result);

/* ------------------- CPU functions --------------------------- */

/**
 * @brief Get raw CPU counters from /proc/stat
 * @param snapshot Pointer to store raw counters
 * @return 0 on success, -1 on error
 */
int sl_cpu_get_raw(sl_cpu_raw_t *snapshot);

/**
 * @brief Calculate CPU usage percentages between two snapshots
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

/* ------------------- Memory functions ------------------------ */

/**
 * @brief Calculate derived memory statistics (used memory, percent)
 * @param result Memory info structure with raw fields populated
 * @return 0 on success, -1 on error
 */
int sl_mem_calculate(sl_mem_info_t *result);

/**
 * @brief Get memory information from /proc/meminfo
 * @param result Pointer to store memory information
 * @return 0 on success, or -1 on error
 */
int sl_mem_get_info(sl_mem_info_t *result);

/* ------------------- Storage functions ----------------------- */

/**
 * @brief Get filesystem storage information for a mounted path
 * @param path Filesystem path
 * @param result Pointer to store storage information
 * @return 0 on success, -1 on error
 */
int sl_storage_get_info(const char *path, sl_storage_info_t *result);


/* ----------------------- Logging ----------------------------- */
/* Log  levels for library message */
typedef enum {
    SL_LOG_INFO,
    SL_LOG_WARN,
    SL_LOG_ERROR
} sl_log_level_t;

/* Log callback type */
typedef void (*sl_log_handler_t)(sl_log_level_t level, const char* func, const char* msg, void* user_data);

/**
 * @brief Set library-wide log handler
 * @param handler User-defined callback function, or NULL to disable logging
 * @param user_data User data pointer passed to callback
 */
void sl_set_log_handler(sl_log_handler_t handler, void* user_data);


#ifdef __cplusplus
}
#endif

#endif