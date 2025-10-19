/**
 * Example usage of sysload library.
 * Demonstrates monitoring CPU, memory, storage, and system uptime.
 * Compile with: make example
 * Run with: ./build/main
 */

#include "sysload.h"
#include <stdio.h>
#include <inttypes.h>

int main() {
    printf("=== sysload Example: System Monitoring ===\n\n");

    // --- CPU Usage Monitoring ---
    printf("--- CPU Statistics (3-second interval) ---\n");
    sl_cpu_usage_t cpu_usage;
    int cpu_ret = sl_cpu_get_usage(3.0f, &cpu_usage);
    if (cpu_ret == 0) {
        printf("User:     %.2f%%\n", cpu_usage.user);
        printf("Nice:     %.2f%%\n", cpu_usage.nice);
        printf("System:   %.2f%%\n", cpu_usage.system);
        printf("Idle:     %.2f%%\n", cpu_usage.idle);
        printf("I/O Wait: %.2f%%\n", cpu_usage.iowait);
        printf("IRQ:      %.2f%%\n", cpu_usage.irq);
        printf("SoftIRQ:  %.2f%%\n", cpu_usage.softirq);
        printf("Steal:    %.2f%%\n", cpu_usage.steal);
        printf("Total:    %.2f%%\n", cpu_usage.total);
    } else {
        printf("Warning: Failed to get CPU usage\n");
    }

    // --- Memory and Swap Monitoring ---
    printf("\n--- Memory Statistics ---\n");
    sl_mem_info_t mem_info;
    int mem_ret = sl_mem_get_info(&mem_info);
    if (mem_ret >= 0) {
        printf("Total:     %"SCNu64" MB\n", sl_kb_to_mb(mem_info.total));
        printf("Free:      %"SCNu64" MB\n", sl_kb_to_mb(mem_info.free));
        printf("Available: %"SCNu64" MB\n", sl_kb_to_mb(mem_info.available));
        printf("Buffers:   %"SCNu64" MB\n", sl_kb_to_mb(mem_info.buffers));
        printf("Cached:    %"SCNu64" MB\n", sl_kb_to_mb(mem_info.cached));
        printf("Shared:    %"SCNu64" MB\n", sl_kb_to_mb(mem_info.shared));
        printf("Used:      %"SCNu64" MB\n", sl_kb_to_mb(mem_info.used));
        printf("Used %%:    %.2f%%\n", mem_info.percent_used);
        printf("Swap Total: %"SCNu64" MB\n", sl_kb_to_mb(mem_info.swap_total));
        printf("Swap Free:  %"SCNu64" MB\n", sl_kb_to_mb(mem_info.swap_free));
        printf("Swap Used:  %"SCNu64" MB\n", sl_kb_to_mb(mem_info.swap_used));
        if (mem_ret < EXPECTED_MEMINFO_KEYS) {
            printf("Warning: Missing %d memory fields from /proc/meminfo\n", EXPECTED_MEMINFO_KEYS - mem_ret);
        }
    } else {
        printf("Error: Failed to get memory info (return code: %d)\n", mem_ret);
    }

    // --- Storage Monitoring (root filesystem) ---
    printf("\n--- Storage Statistics (Root FS /) ---\n");
    sl_storage_info_t rootfs_info;
    int storage_ret = sl_storage_get_info("/", &rootfs_info);
    if (storage_ret == 0 && rootfs_info.total > 0) {
        printf("Total:     %"SCNu64" GB\n", sl_bytes_to_gb(rootfs_info.total));
        printf("Available: %"SCNu64" GB\n", sl_bytes_to_gb(rootfs_info.available));
        printf("Used:      %"SCNu64" GB\n", sl_bytes_to_gb(rootfs_info.used));
        printf("Used %%:    %.2f%%\n", rootfs_info.percent_usage);
    } else {
        printf("Warning: Failed to get storage info\n");
    }

    // --- System Uptime ---
    printf("\n--- System Uptime ---\n");
    sl_systime_info_t time_info;
    int uptime_ret = sl_systime_get_info(&time_info);
    if (uptime_ret == 0) {
        printf("Uptime:    %.2lf seconds\n", time_info.uptime);
        printf("Idle time: %.2lf seconds\n", time_info.idle_time);
    } else {
        printf("Warning: Failed to get uptime info\n");
    }

    printf("\n=== Monitoring complete ===\n");
    return 0;
}