## 檔案說明
- 整合前: 1, 2, 3部分合併前檔案（parser, analyzer）
- APB_Recognizer.exe: 執行檔
- main.c: 主程式
- dump00.vcd: 測試輸入檔案
- dump00.txt: 測試輸出檔案

## 已完成
- APB 交易辨識（第一部分）
- APB Bus 行為分析（第二部分）
- Requester & Completer辨識（第三部分）

## 未完成 
- Completer 錯誤類型分析（算第三還是第四部分？）
- APB 錯誤檢測（第四部份）

## 使用方式
把dump檔案放在相同目錄，並在終端機輸入
```bash
./APB_Recognizer dump00.vcd -o dump00.txt
```
目前測試結果：
- 輸入: dump00.vcd (pulpino_public_testcase3.vcd)
- 輸出: dump00.txt  
Number of Read Transactions with no wait states: 300  
Number of Read Transactions with wait states: 0  
Number of Write Transactions with no wait states: 606  
Number of Write Transactions with wait states: 0  
Average Read Cycle: 2.00 cycles  
Average Write Cycle: 2.00 cycles  
Bus Utilization: 34.12%  
Number of Idle Cycles: 3498  
Number of Requester: 1  
Number of Completer: 1  
CPU Elapsed Time: 9.00 ms  

## 備註
會繼續用branch對程式做測試，有確定的變更會merge到main branch