#include <stdio.h>

// 交易型態：READ 或 WRITE
typedef enum { READ, WRITE } TransactionType;

// APB 交易結構
typedef struct {
    TransactionType type;
    int wait_state; // 0: 無wait state, 1: 有wait state
} APB_Transaction;

// 根據VCD範例手動捏造的交易資料
APB_Transaction transactions[] = {
    {READ, 0},
    {READ, 0},
    {WRITE, 0},
    {READ, 0},
    {WRITE, 0},
    {READ, 0},
    {READ, 0},
    {WRITE, 0},
    {WRITE, 0},
    {READ, 0},
    {WRITE, 0},
    {READ, 0},
    {READ, 0},
    {WRITE, 0},
    {READ, 0},
    {READ, 0},
    {WRITE, 0},
    {READ, 0},
    {WRITE, 0},
};

// 交易總數
int num_transactions = sizeof(transactions) / sizeof(transactions[0]); 