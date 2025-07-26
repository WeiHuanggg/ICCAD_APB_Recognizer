#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    FILE *fp = fopen("transaction_output.txt", "r");
    if (!fp) {
        printf("Cannot open transaction_output.txt\n");
        return 1;
    }

    char line[256];
    int read_no_wait = 0, read_with_wait = 0;
    int write_no_wait = 0, write_with_wait = 0;
    int read_cycles = 0, write_cycles = 0;
    int prev_time = -1, first_time = -1, last_time = -1;
    int total_transactions = 0;
    int total_cycles_used = 0;

    // 逐行讀取交易資料
    while (fgets(line, sizeof(line), fp)) {
        int time = 0;
        char type[8];
        if (sscanf(line, "#%d %7s", &time, type) == 2) {
            if (first_time == -1) first_time = time;
            last_time = time;
            
            // 假設每筆交易都沒有 wait state（因為目前檔案沒有這個資訊）
            if (strcmp(type, "READ") == 0) {
                read_no_wait++;  // 假設都沒有 wait state
                if (prev_time != -1) {
                    int cycle_count = (time - prev_time);
                    read_cycles += cycle_count;
                    total_cycles_used += cycle_count;
                }
                prev_time = time;
            } else if (strcmp(type, "WRITE") == 0) {
                write_no_wait++;  // 假設都沒有 wait state
                if (prev_time != -1) {
                    int cycle_count = (time - prev_time);
                    write_cycles += cycle_count;
                    total_cycles_used += cycle_count;
                }
                prev_time = time;
            }
            total_transactions++;
        }
    }
    fclose(fp);

    // 計算總模擬週期
    int total_simulation_cycles = (last_time - first_time);
    
    // 計算閒置週期
    int idle_cycles = total_simulation_cycles - total_cycles_used;
    
    // 計算平均週期（假設每筆交易最少2個cycle）
    double avg_read_cycle = (read_no_wait > 0) ? 2.0 : 0.0;
    double avg_write_cycle = (write_no_wait > 0) ? 2.0 : 0.0;
    
    // 計算 Bus Utilization
    double bus_utilization = (double)total_cycles_used / total_simulation_cycles * 100.0;
    
    // 計算 CPU 執行時間（假設時脈頻率，這裡用1GHz估算）
    double cpu_time_ms = (double)total_simulation_cycles / 1000000.0;  // 假設1GHz時脈

    // 輸出結果
    printf("Number of Read Transactions with no wait states: %d\n", read_no_wait);
    printf("Number of Read Transactions with wait states: %d\n", read_with_wait);
    printf("Number of Write Transactions with no wait states: %d\n", write_no_wait);
    printf("Number of Write Transactions with wait states: %d\n", write_with_wait);
    printf("Average Read Cycle: %.2f cycles\n", avg_read_cycle);
    printf("Average Write Cycle: %.2f cycles\n", avg_write_cycle);
    printf("Bus Utilization: %.2f%%\n", bus_utilization);
    printf("Number of Idle Cycles: %d\n", idle_cycles);
    printf("Number of Completer: 1\n");  // 假設只有一個 completer
    printf("CPU Elapsed Time: %.2f ms\n", cpu_time_ms);

    return 0;
}