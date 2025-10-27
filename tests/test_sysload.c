#include "sysload.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int total = 0, passed = 0;

#define OK      "[ \033[32m OK \033[0m ]"
#define FAIL    "[ \033[31mFAIL\033[0m ]"


#define TEST_EQ(expr, expected) do { \
        total++; \
        int _r = (expr); \
        if (_r == (expected)) { \
                printf(OK " %s returned %d\n", #expr, _r); \
                passed++; \
        } else { \
                fprintf(stderr, FAIL " %s returned %d (expected %d)\n", #expr, _r, expected); \
        } \
} while(0)

#define TEST_RANGE(value, min, max) do { \
        total++; \
        if ((value) >= (min) && (value) <= (max)) { \
                printf(OK " %s = %.2f (in range)\n", #value, (double)(value)); \
                passed++; \
        } else { \
                fprintf(stderr, FAIL " %s = %.2f (out of range %.2f..%.2f)\n", #value, (double)(value), (double)(min), (double)(max)); \
        } \
} while(0)


int main(void)
{
        printf("\n=== sysload self-test ===\n");

        sl_cpu_usage_t cpu;
        TEST_EQ(sl_cpu_get_usage(1.0f, &cpu), 0);
        TEST_RANGE(cpu.total, 0.0, 100.0);
        TEST_EQ(sl_cpu_get_usage(0.01f, &cpu), -1);
        TEST_EQ(sl_cpu_get_usage(1.0f, NULL), -1);

        sl_mem_info_t mem;
        TEST_EQ(sl_mem_get_info(&mem), 0);
        TEST_RANGE(mem.percent_used, 0.0, 100.0);
        TEST_EQ(sl_mem_get_info(NULL), -1);

        sl_storage_info_t storage; 
        TEST_EQ(sl_storage_get_info("/", &storage), 0);
        TEST_EQ(sl_storage_get_info("/notexistfolder", &storage), -1);
        TEST_EQ(sl_storage_get_info(NULL, NULL), -1);

        sl_systime_info_t systime;
        TEST_EQ(sl_systime_get_info(&systime), 0);
        TEST_RANGE(systime.uptime, 0.1, 1e9);
        TEST_EQ(sl_systime_get_info(NULL), -1);

        printf("\n--- Summary ---\n"); 
        printf("Passed: %d / %d\n", passed, total);
        printf("Result: %s\n", (passed == total) ? "\033[32mSUCCESS\033[0m" : "\033[31mFAIL\033[0m");
        return (passed == total) ? 0 : -1;
}