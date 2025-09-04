#include "sysload.h"
#include <stdio.h>
#include <unistd.h>


int _get_cpu_times(cpu_stat_t *timesptr)
{
        FILE* fptr;
        fptr = fopen("/proc/stat", "r");
        
        if (fptr == NULL) {
                perror("Can't open /proc/stat file\n");
                return -1;
        }

        if (fscanf(fptr, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &timesptr->user,
                   &timesptr->user_nice,
                   &timesptr->system,
                   &timesptr->idle,
                   &timesptr->iowait,
                   &timesptr->irq,
                   &timesptr->softirq,
                   &timesptr->steal,
                   &timesptr->guest,
                   &timesptr->guest_nice) == 10) {
                fclose(fptr);
                return 0;
        } else { 
                fclose(fptr);
                perror("Can't read /proc/stat file\n");
                return -1;
        }
}


float get_cpu_load(int measurement_inteval)
{
        cpu_stat_t start_stat, end_stat;
        uint64_t total_start, total_end, idle_start, idle_end;
        uint64_t total_diff, idle_diff;
        float result;

        _get_cpu_times(&start_stat);
        sleep(measurement_inteval);
        _get_cpu_times(&end_stat);

        total_start = start_stat.user 
                    + start_stat.user_nice
                    + start_stat.system 
                    + start_stat.idle 
                    + start_stat.iowait 
                    + start_stat.irq 
                    + start_stat.softirq
                    + start_stat.steal;
    
        total_end = end_stat.user
                  + end_stat.user_nice 
                  + end_stat.system 
                  + end_stat.idle 
                  + end_stat.iowait 
                  + end_stat.irq 
                  + end_stat.softirq 
                  + end_stat.steal;


        idle_start = start_stat.idle + start_stat.iowait;
        idle_end = end_stat.idle + end_stat.iowait;

        total_diff = total_end - total_start;
        idle_diff = idle_end - idle_start;

        result = ((float)(total_diff - idle_diff) / total_diff) * 100.0f;

        return result;
}

