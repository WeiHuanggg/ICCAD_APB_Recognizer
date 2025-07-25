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
    int read_count = 0, write_count = 0;
    int read_cycles = 0, write_cycles = 0;
    int prev_time = -1, first_time = -1, last_time = -1;
    int total_transactions = 0;

    // 先假設每筆交易都沒有wait state（即兩筆交易之間的時間差就是這筆交易的週期）
    while (fgets(line, sizeof(line), fp)) {
        int time = 0;
        char type[8];
        if (sscanf(line, "#%d %7s", &time, type) == 2) {
            if (first_time == -1) first_time = time;
            last_time = time;
            if (strcmp(type, "READ") == 0) {
                read_count++;
                if (prev_time != -1) read_cycles += (time - prev_time);
                prev_time = time;
            } else if (strcmp(type, "WRITE") == 0) {
                write_count++;
                if (prev_time != -1) write_cycles += (time - prev_time);
                prev_time = time;
            }
            total_transactions++;
        }
    }
    fclose(fp);

    // 平均週期
    double avg_read_cycle = (read_count > 1) ? (double)read_cycles / (read_count - 1) : 0.0;
    double avg_write_cycle = (write_count > 1) ? (double)write_cycles / (write_count - 1) : 0.0;

    // 總模擬週期
    int total_cycles = (last_time - first_time);

    // Bus Utilization: 交易數 * 2 / 總模擬週期（假設每筆交易最少2cycle）
    double bus_utilization = (total_transactions * 2.0) / total_cycles * 100.0;

    printf("Number of Read Transactions: %d\n", read_count);
    printf("Number of Write Transactions: %d\n", write_count);
    printf("Average Read Cycle: %.2f cycles\n", avg_read_cycle);
    printf("Average Write Cycle: %.2f cycles\n", avg_write_cycle);
    printf("Bus Utilization: %.2f%%\n", bus_utilization);

    return 0;
}