#include <stdio.h>
#include "parserResult.c"

int main() {
    int read_no_wait = 0;
    int read_with_wait = 0;
    int write_no_wait = 0;
    int write_with_wait = 0;

    int total_read_cycles = 0;
    int total_write_cycles = 0;
    int total_transaction_cycles = 0;

    // 假設總模擬週期數（可根據實際VCD內容調整）
    int total_cycles = 100000;

    // 逐筆檢查交易，統計各類型數量與週期
    for (int i = 0; i < num_transactions; i++) {
        int cycles = 2 + transactions[i].wait_state; // APB每筆最少2cycle，加上wait state
        total_transaction_cycles += cycles;

        if (transactions[i].type == READ) {
            total_read_cycles += cycles;
            if (transactions[i].wait_state == 0)
                read_no_wait++;
            else
                read_with_wait++;
        } else if (transactions[i].type == WRITE) {
            total_write_cycles += cycles;
            if (transactions[i].wait_state == 0)
                write_no_wait++;
            else
                write_with_wait++;
        }
    }

    // 計算平均週期
    double avg_read_cycle = (read_no_wait + read_with_wait) ? (double)total_read_cycles / (read_no_wait + read_with_wait) : 0.0;
    double avg_write_cycle = (write_no_wait + write_with_wait) ? (double)total_write_cycles / (write_no_wait + write_with_wait) : 0.0;

    // 計算Bus Utilization
    double bus_utilization = (double)total_transaction_cycles / total_cycles * 100.0;

    // 輸出結果
    printf("Number of Read Transactions with no wait states: %d\n", read_no_wait);
    printf("Number of Read Transactions with wait states: %d\n", read_with_wait);
    printf("Number of Write Transactions with no wait states: %d\n", write_no_wait);
    printf("Number of Write Transactions with wait states: %d\n", write_with_wait);

    printf("Average Read Cycle: %.2f cycles\n", avg_read_cycle);
    printf("Average Write Cycle: %.2f cycles\n", avg_write_cycle);
    printf("Bus Utilization: %.2f%%\n", bus_utilization);

    return 0;
}
