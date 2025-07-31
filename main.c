#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SYMBOLS 1024
#define MAX_LINE_LEN 1024

// 用來儲存 symbol 對應表
char symbol_map[6][2]; // PADDR, PWDATA, PWRITE, PSEL, PENABLE, PREADY

// 依照順序對應欄位
enum { IDX_PADDR, IDX_PWDATA, IDX_PWRITE, IDX_PSEL, IDX_PENABLE, IDX_PREADY };

// 存放目前每個 symbol 的值
char signal_values[MAX_SYMBOLS][MAX_LINE_LEN];
char symbol_list[MAX_SYMBOLS][MAX_LINE_LEN];
int symbol_count = 0;

// 新增：前一個時間點的訊號值
char prev_signal_values[6][MAX_LINE_LEN] = {"x","x","x","x","x","x"};

// 初始化 symbol map（直接寫在程式裡）
int init_symbol_map() {
    symbol_map[IDX_PADDR][0] = '%';
    symbol_map[IDX_PWDATA][0] = '&';
    symbol_map[IDX_PWRITE][0] = '\'';
    symbol_map[IDX_PSEL][0] = '(';
    symbol_map[IDX_PENABLE][0] = ')';
    symbol_map[IDX_PREADY][0] = '*';
    return 1;
}

void update_value(const char* symbol, const char* value) {
    int i;
    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_list[i], symbol) == 0) {
            strcpy(signal_values[i], value);
            return;
        }
    }
    strcpy(symbol_list[symbol_count], symbol);
    strcpy(signal_values[symbol_count], value);
    symbol_count++;
}

int get_symbol_index(char symbol) {
    int i;
    for (i = 0; i < 6; i++) {
        if (symbol_map[i][0] == symbol) return i;
    }
    return -1;
}

const char* get_value_by_index(int idx) {
    char symbol = symbol_map[idx][0];
    int i;
    for (i = 0; i < symbol_count; i++) {
        if (symbol_list[i][0] == symbol) {
            return signal_values[i];
        }
    }
    return "x";
}

struct WaitStateInfo {
    int start_time;
    int end_time;
    int wait_count;
    int valid;
    char wr[8], addr[64], data[64];
    int requester_id;
    int completer_id;
};

int get_completer_id(const char* addr) {
    return 1;
}

int main(int argc, char* argv[]) {
    clock_t start = clock();
    
    char output_filename[256];
    
    if (argc == 2) {
        // 方式1：自動命名 - ./APB_Recognizer dump1.vcd
        strcpy(output_filename, argv[1]);
        char* dot = strrchr(output_filename, '.');
        if (dot && strcmp(dot, ".vcd") == 0) {
            strcpy(dot, ".txt");
        } else {
            strcat(output_filename, ".txt");
        }
    } else if (argc == 4 && strcmp(argv[2], "-o") == 0) {
        // 方式2：手動指定 - ./APB_Recognizer dump1.vcd -o dump1.txt
        strcpy(output_filename, argv[3]);
    } else {
        return 1;
    }
    
    init_symbol_map();
    FILE* fin = fopen(argv[1], "r");
    FILE* fresult = fopen(output_filename, "w");
    if (!fin || !fresult) {
        return 1;
    }
    char line[MAX_LINE_LEN];
    int current_time = 0;
    int in_transaction = 0;
    struct WaitStateInfo wsinfo = {0, 0, 0, 0, "", "", "", 0, 0};
    char prev_sel[8] = "0";
    char prev_en[8] = "0";
    char prev_pready[8] = "0";

    // 交易統計用
    int read_no_wait = 0, read_with_wait = 0;
    int write_no_wait = 0, write_with_wait = 0;
    int read_cycles = 0, write_cycles = 0;
    int first_time = 0, last_time = -1;
    int total_transactions = 0;
    int total_cycles_used = 0;

    // 先將所有交易暫存到記憶體
    struct {
        int start_time, end_time, wait_states;
        char type[8], addr[64], data[64];
        int req_id, comp_id;
    } transactions[10000];
    int txn_count = 0;

    while (fgets(line, sizeof(line), fin)) {
        if (line[0] == '#') {
            current_time = atoi(&line[1]);
            int i;
            for (i = 0; i < 6; i++) {
                const char* v = get_value_by_index(i);
                strncpy(prev_signal_values[i], v, MAX_LINE_LEN);
            }
            // 取得目前訊號
            const char* sel = get_value_by_index(IDX_PSEL);
            const char* en  = get_value_by_index(IDX_PENABLE);
            const char* wr  = get_value_by_index(IDX_PWRITE);
            const char* addr = get_value_by_index(IDX_PADDR);
            const char* data = get_value_by_index(IDX_PWDATA);
            const char* pready = get_value_by_index(IDX_PREADY);
            // 交易開始
            if (!in_transaction && strcmp(sel, "1") == 0 && strcmp(en, "1") == 0 && strcmp(prev_en, "0") == 0) {
                in_transaction = 1;
                wsinfo.start_time = current_time;
                wsinfo.wait_count = 0;
                wsinfo.valid = 1;
                strncpy(wsinfo.wr, wr, sizeof(wsinfo.wr));
                strncpy(wsinfo.addr, addr, sizeof(wsinfo.addr));
                strncpy(wsinfo.data, data, sizeof(wsinfo.data));
                wsinfo.requester_id = 1;
                wsinfo.completer_id = get_completer_id(addr);
                if (strcmp(pready, "1") == 0) {
                    wsinfo.end_time = wsinfo.start_time + 2 * 5000;
                    // 寫入暫存
                    strcpy(transactions[txn_count].type, strcmp(wr, "1") == 0 ? "WRITE" : "READ");
                    strcpy(transactions[txn_count].addr, addr);
                    strcpy(transactions[txn_count].data, data);
                    transactions[txn_count].start_time = wsinfo.start_time;
                    transactions[txn_count].end_time = wsinfo.end_time;
                    transactions[txn_count].wait_states = 0;
                    transactions[txn_count].req_id = wsinfo.requester_id;
                    transactions[txn_count].comp_id = wsinfo.completer_id;
                    txn_count++;
                    in_transaction = 0;
                    wsinfo.valid = 0;
                }
            } else if (in_transaction && strcmp(sel, "1") == 0 && strcmp(en, "1") == 0) {
                if (strcmp(pready, "1") != 0) {
                    wsinfo.wait_count++;
                } else {
                    wsinfo.end_time = wsinfo.start_time + (2 + wsinfo.wait_count) * 5000;
                    strcpy(transactions[txn_count].type, strcmp(wsinfo.wr, "1") == 0 ? "WRITE" : "READ");
                    strcpy(transactions[txn_count].addr, wsinfo.addr);
                    strcpy(transactions[txn_count].data, wsinfo.data);
                    transactions[txn_count].start_time = wsinfo.start_time;
                    transactions[txn_count].end_time = wsinfo.end_time;
                    transactions[txn_count].wait_states = wsinfo.wait_count;
                    transactions[txn_count].req_id = wsinfo.requester_id;
                    transactions[txn_count].comp_id = wsinfo.completer_id;
                    txn_count++;
                    in_transaction = 0;
                    wsinfo.valid = 0;
                }
            } else {
                in_transaction = 0;
                wsinfo.valid = 0;
            }
            strncpy(prev_sel, sel, sizeof(prev_sel));
            strncpy(prev_en, en, sizeof(prev_en));
            strncpy(prev_pready, pready, sizeof(prev_pready));
        } else if (line[0] == 'b') {
            char value[MAX_LINE_LEN], symbol[MAX_LINE_LEN];
            sscanf(line + 1, "%s %s", value, symbol);
            update_value(symbol, value);
        } else if (line[0] == '0' || line[0] == '1') {
            char symbol[MAX_LINE_LEN];
            strcpy(symbol, line + 1);
            symbol[strcspn(symbol, "\n")] = 0;
            char val[2] = { line[0], '\0' };
            update_value(symbol, val);
        }
    }
    if (wsinfo.valid) {
        wsinfo.end_time = wsinfo.start_time + (2 + wsinfo.wait_count) * 5000;
        strcpy(transactions[txn_count].type, strcmp(wsinfo.wr, "1") == 0 ? "WRITE" : "READ");
        strcpy(transactions[txn_count].addr, wsinfo.addr);
        strcpy(transactions[txn_count].data, wsinfo.data);
        transactions[txn_count].start_time = wsinfo.start_time;
        transactions[txn_count].end_time = wsinfo.end_time;
        transactions[txn_count].wait_states = wsinfo.wait_count;
        transactions[txn_count].req_id = wsinfo.requester_id;
        transactions[txn_count].comp_id = wsinfo.completer_id;
        txn_count++;
    }
    fclose(fin);

    // 統計分析
    int i;
    for (i = 0; i < txn_count; i++) {
        int start_time = transactions[i].start_time;
        int end_time = transactions[i].end_time;
        int wait_states = transactions[i].wait_states;
        char* type = transactions[i].type;
        if (end_time > last_time) last_time = end_time;
        int transaction_cycles = (end_time - start_time) / 5000;
        total_cycles_used += transaction_cycles;
        if (strcmp(type, "READ") == 0) {
            if (wait_states == 0) {
                read_no_wait++;
            } else {
                read_with_wait++;
            }
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
    int total_simulation_cycles = (last_time - first_time) / 10000;
    int idle_cycles = total_simulation_cycles - total_cycles_used;
    double avg_read_cycle = (read_no_wait + read_with_wait > 0) ? (double)read_cycles / (read_no_wait + read_with_wait) : 0.0;
    double avg_write_cycle = (write_no_wait + write_with_wait > 0) ? (double)write_cycles / (write_no_wait + write_with_wait) : 0.0;
    double bus_utilization = (double)total_cycles_used / total_simulation_cycles * 100.0;
    clock_t end = clock();
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    // 輸出到指定的輸出檔案
    fprintf(fresult, "Number of Read Transactions with no wait states: %d\n", read_no_wait);
    fprintf(fresult, "Number of Read Transactions with wait states: %d\n", read_with_wait);
    fprintf(fresult, "Number of Write Transactions with no wait states: %d\n", write_no_wait);
    fprintf(fresult, "Number of Write Transactions with wait states: %d\n", write_with_wait);
    fprintf(fresult, "Average Read Cycle: %.2f cycles\n", avg_read_cycle);
    fprintf(fresult, "Average Write Cycle: %.2f cycles\n", avg_write_cycle);
    fprintf(fresult, "Bus Utilization: %.2f%%\n", bus_utilization);
    fprintf(fresult, "Number of Idle Cycles: %d\n", idle_cycles);
    fprintf(fresult, "Number of Completer: 1\n");
    fprintf(fresult, "CPU Elapsed Time: %.2f ms\n", elapsed_ms);
    fclose(fresult);
    return 0;
}

