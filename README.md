## 檔案說明
- main.c: 主程式
- APB_Recognizer.exe: 執行檔
- 整合前: 1, 2, 3部分合併前檔案（parser, analyzer）

## 已完成
- APB 交易辨識
- APB Bus 行為分析
- Requester & Completer辨識

## 未完成 
- Completer 錯誤類型分析（？
- APB 錯誤檢測

## 使用方式
```bash
./APB_Recognizer dump00.vcd -o dump00.txt
```

## 備註
會繼續用branch對程式做實驗，有確定的變更會merge到main branch