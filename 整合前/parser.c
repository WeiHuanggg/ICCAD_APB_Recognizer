#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// 讀取 symbol_map.txt
int load_symbol_map(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Cannot open symbol_map.txt");
        return 0;
    }

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        char key[32], value;
        if (sscanf(line, "%[^=]=%c", key, &value) == 2) {
            if (strcmp(key, "PADDR") == 0) symbol_map[IDX_PADDR][0] = value;
            else if (strcmp(key, "PWDATA") == 0) symbol_map[IDX_PWDATA][0] = value;
            else if (strcmp(key, "PWRITE") == 0) symbol_map[IDX_PWRITE][0] = value;
            else if (strcmp(key, "PSEL") == 0) symbol_map[IDX_PSEL][0] = value;
            else if (strcmp(key, "PENABLE") == 0) symbol_map[IDX_PENABLE][0] = value;
            else if (strcmp(key, "PREADY") == 0) symbol_map[IDX_PREADY][0] = value;
        }
    }

    fclose(fp);
    return 1;
}

void update_value(const char* symbol, const char* value) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_list[i], symbol) == 0) {
            strcpy(signal_values[i], value);
            return;
        }
    }
    // 若還沒出現過 symbol，新增一筆
    strcpy(symbol_list[symbol_count], symbol);
    strcpy(signal_values[symbol_count], value);
    symbol_count++;
}

// 取得 symbol 對應的 index
int get_symbol_index(char symbol) {
    for (int i = 0; i < 6; i++) {
        if (symbol_map[i][0] == symbol) return i;
    }
    return -1;
}

// 取得目前訊號值（依 index）
const char* get_value_by_index(int idx) {
    char symbol = symbol_map[idx][0];
    for (int i = 0; i < symbol_count; i++) {
        if (symbol_list[i][0] == symbol) {
            return signal_values[i];
        }
    }
    return "x";
}

// 新增 wait state 計算結構，增加結束時間和ID資訊
struct WaitStateInfo {
    int start_time;
    int end_time;      // 新增：交易結束時間
    int wait_count;
    int valid;
    char wr[8], addr[64], data[64];
    int requester_id;  // 新增：Requester ID
    int completer_id;  // 新增：Completer ID
};

struct WaitStateInfo wsinfo = {0, 0, 0, 0, "", "", "", 0, 0};

// 根據地址判斷 Completer ID（簡單的地址範圍判斷）
int get_completer_id(const char* addr) {
    // 這裡可以根據實際的地址映射來判斷
    // 目前假設所有交易都到同一個 Completer
    return 1;
}

// 交易偵測：只在 PENABLE 由 0->1 且 PSEL==1 時觸發，wait state 由主迴圈追蹤
void output_transaction(FILE* ftrans, int time, const char* wr, const char* addr, const char* data, int wait_count, int end_time, int requester_id, int completer_id) {
    fprintf(ftrans, "#%d ", time);
    if (strcmp(wr, "1") == 0)
        fprintf(ftrans, "WRITE  ");
    else
        fprintf(ftrans, "READ   ");
    fprintf(ftrans, "ADDR=%s DATA=%s WAIT=%d END=%d REQ_ID=%d COMP_ID=%d\n", 
            addr, data, wait_count, end_time, requester_id, completer_id);
}

int main(int argc, char* argv[]) {
    if (argc < 4 || strcmp(argv[2], "-o") != 0) {
        printf("Usage: %s <input.vcd> -o <output.txt>\n", argv[0]);
        return 1;
    }

    if (!load_symbol_map("symbol_map.txt")) {
        return 1;
    }

    FILE* fin = fopen(argv[1], "r");
    FILE* fout = fopen(argv[3], "w");
    FILE* ftrans = fopen("transaction_output.txt", "w");
    if (!fin || !fout || !ftrans) {
        perror("File open error");
        return 1;
    }

    char line[MAX_LINE_LEN];
    int current_time = 0;
    int first_time = 1;
    // 新增：wait state 追蹤 flag
    int in_transaction = 0;
    wsinfo.valid = 0;
    
    // 新增：追蹤前一個時間點的訊號值
    char prev_sel[8] = "0";
    char prev_en[8] = "0";
    char prev_pready[8] = "0";
    
    while (fgets(line, sizeof(line), fin)) {
        if (line[0] == '#') {
            // 在進入新時間點前，先判斷上一個時間點是否有交易
            // wait state 判斷在主迴圈處理
            // 更新時間
            current_time = atoi(&line[1]);
            // 儲存目前訊號值到 prev_signal_values
            for (int i = 0; i < 6; i++) {
                const char* v = get_value_by_index(i);
                strncpy(prev_signal_values[i], v, MAX_LINE_LEN);
            }
            fprintf(fout, "%s", line);

            // 取得目前訊號
            const char* sel = get_value_by_index(IDX_PSEL);
            const char* en  = get_value_by_index(IDX_PENABLE);
            const char* wr  = get_value_by_index(IDX_PWRITE);
            const char* addr = get_value_by_index(IDX_PADDR);
            const char* data = get_value_by_index(IDX_PWDATA);
            const char* pready = get_value_by_index(IDX_PREADY);

            // 交易開始：psel=1, penable 由 0->1（避免重複觸發）
            if (!in_transaction && strcmp(sel, "1") == 0 && strcmp(en, "1") == 0 && strcmp(prev_en, "0") == 0) {
                in_transaction = 1;
                wsinfo.start_time = current_time;
                wsinfo.wait_count = 0;
                wsinfo.valid = 1;
                strncpy(wsinfo.wr, wr, sizeof(wsinfo.wr));
                strncpy(wsinfo.addr, addr, sizeof(wsinfo.addr));
                strncpy(wsinfo.data, data, sizeof(wsinfo.data));
                wsinfo.requester_id = 1;  // 假設只有一個 Requester
                wsinfo.completer_id = get_completer_id(addr);
                // 如果 pready=1，馬上結束（結束時間為setup+access）
                if (strcmp(pready, "1") == 0) {
                    wsinfo.end_time = wsinfo.start_time + 2 * 5000;
                    output_transaction(ftrans, wsinfo.start_time, wr, addr, data, 0, 
                                     wsinfo.end_time, wsinfo.requester_id, wsinfo.completer_id);
                    in_transaction = 0;
                    wsinfo.valid = 0;
                }
            } else if (in_transaction && strcmp(sel, "1") == 0 && strcmp(en, "1") == 0) {
                // 交易進行中，pready=0 則 wait state 累加
                if (strcmp(pready, "1") != 0) {
                    wsinfo.wait_count++;
                } else {
                    // pready=1，交易完成（結束時間為setup+access+wait）
                    wsinfo.end_time = wsinfo.start_time + (2 + wsinfo.wait_count) * 5000;
                    output_transaction(ftrans, wsinfo.start_time, wsinfo.wr, wsinfo.addr, wsinfo.data, 
                                     wsinfo.wait_count, wsinfo.end_time, wsinfo.requester_id, wsinfo.completer_id);
                    in_transaction = 0;
                    wsinfo.valid = 0;
                }
            } else {
                // 其他情況，reset 狀態
                in_transaction = 0;
                wsinfo.valid = 0;
            }
            
            // 更新前一個時間點的訊號值
            strncpy(prev_sel, sel, sizeof(prev_sel));
            strncpy(prev_en, en, sizeof(prev_en));
            strncpy(prev_pready, pready, sizeof(prev_pready));
            
        } else if (line[0] == 'b') {
            char value[MAX_LINE_LEN], symbol[MAX_LINE_LEN];
            sscanf(line + 1, "%s %s", value, symbol);
            update_value(symbol, value);
            fprintf(fout, "%s", line);
        } else if (line[0] == '0' || line[0] == '1') {
            char symbol[MAX_LINE_LEN];
            strcpy(symbol, line + 1);
            symbol[strcspn(symbol, "\n")] = 0;
            char val[2] = { line[0], '\0' };
            update_value(symbol, val);
            fprintf(fout, "%s", line);
        }
    }
    // 最後一個時間點若有未完成交易，強制輸出
    if (wsinfo.valid) {
        wsinfo.end_time = wsinfo.start_time + (2 + wsinfo.wait_count) * 5000;
        output_transaction(ftrans, wsinfo.start_time, wsinfo.wr, wsinfo.addr, wsinfo.data, 
                         wsinfo.wait_count, wsinfo.end_time, wsinfo.requester_id, wsinfo.completer_id);
    }

    fclose(fin);
    fclose(fout);
    fclose(ftrans);

    printf("Done. Output: %s, transaction_output.txt\n", argv[3]);
    return 0;
}
