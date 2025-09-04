#ifndef _SYSLOAD_H
#define _SYSLOAD_H

#include <stdint.h>

/**
 * @struct cpu_stat_t
 * @brief CPU time statistics structure
 * 
 * Contains cumulative time values spent by CPU in various states,
 * measured in jiffies (typically 1/100 or 1/1000 of a second)
 */
typedef struct 
{
        uint64_t user,
                 user_nice,
                 system,
                 idle,
                 iowait,
                 irq,
                 softirq,
                 steal, 
                 guest,
                 guest_nice;
} cpu_stat_t;


/**
 * @brief Calculates CPU load over specified time interval
 * 
 * Measures CPU utilization - percentage of time when CPU was busy
 * executing processes (not in idle or iowait state)
 * 
 * @param measurement_interval Measurement interval in seconds
 * @return CPU load percentage (0-100%) or negative value on error
 */
float get_cpu_load(int measurement_inteval);

#endif