## 檔案說明
- main.c: 主程式
- APB_Recognizer.exe: 執行檔
- 整合前: 1, 2, 3部分合併前檔案（parser, analyzer）

## 已完成
- APB 交易辨識（第一部分）
- APB Bus 行為分析（第二部分）
- Requester & Completer辨識（第三部分）

## 未完成 
- Completer 錯誤類型分析（算第三還是第四部分？）
- APB 錯誤檢測（第四部份）

## 使用方式
```bash
./APB_Recognizer dump00.vcd -o dump00.txt
```

## 備註
會繼續用branch對程式做實驗，有確定的變更會merge到main branch