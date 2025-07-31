#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main() {
    clock_t start = clock();
    FILE *fp = fopen("transaction_output.txt", "r");
    if (!fp) {
        printf("Cannot open transaction_output.txt\n");
        return 1;
    }

    char line[256];
    int read_no_wait = 0, read_with_wait = 0;
    int write_no_wait = 0, write_with_wait = 0;
    int read_cycles = 0, write_cycles = 0;
    int first_time = 0, last_time = -1;
    int total_transactions = 0;
    int total_cycles_used = 0;

    // 逐行讀取交易資料
    while (fgets(line, sizeof(line), fp)) {
        int start_time = 0, end_time = 0, wait_states = 0;
        char type[8], addr[64], data[64];
        int req_id, comp_id;
        
        // 解析新格式：#<start_time> <type> ADDR=<addr> DATA=<data> WAIT=<wait> END=<end> REQ_ID=<req> COMP_ID=<comp>
        if (sscanf(line, "#%d %7s ADDR=%s DATA=%s WAIT=%d END=%d REQ_ID=%d COMP_ID=%d", 
                   &start_time, type, addr, data, &wait_states, &end_time, &req_id, &comp_id) >= 6) {
            
            // first_time = 0，last_time取最大end_time
            if (end_time > last_time) last_time = end_time;
            
            // 計算這筆交易的週期（將時間單位轉換為時脈週期，假設時脈週期為5000單位）
            int transaction_cycles = (end_time - start_time) / 5000;
            total_cycles_used += transaction_cycles;
            
            if (strcmp(type, "READ") == 0) {
                if (wait_states == 0) {
                    read_no_wait++;
                } else {
                    read_with_wait++;                }
                read_cycles += transaction_cycles;
            } else if (strcmp(type, "WRITE") == 0) {
                if (wait_states == 0) {
                    write_no_wait++;
                } else {
                    write_with_wait++;
                }
                write_cycles += transaction_cycles;
            }
            total_transactions++;
        }
    }
    fclose(fp);

    // 計算總模擬週期（將時間單位轉換為時脈週期，這裡一個完整時脈週期為10000單位）
    int total_simulation_cycles = (last_time - first_time) / 10000;
    
    // 計算閒置週期
    int idle_cycles = total_simulation_cycles - total_cycles_used;
    
    // 計算平均週期
    double avg_read_cycle = (read_no_wait + read_with_wait > 0) ? 
                           (double)read_cycles / (read_no_wait + read_with_wait) : 0.0;
    double avg_write_cycle = (write_no_wait + write_with_wait > 0) ? 
                            (double)write_cycles / (write_no_wait + write_with_wait) : 0.0;
    
    // 計算 Bus Utilization
    double bus_utilization = (double)total_cycles_used / total_simulation_cycles * 100.0;
    
    // 計算 CPU 執行時間（假設時脈頻率，這裡用1GHz估算）
    // double cpu_time_ms = (double)total_simulation_cycles / 1000000.0;  // 假設1GHz時脈

    clock_t end = clock();
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

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
    printf("CPU Elapsed Time: %.2f ms\n", elapsed_ms);

    return 0;
}