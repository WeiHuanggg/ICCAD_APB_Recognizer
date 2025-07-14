我需要一支Graduate school一年級程度的C語言程式
用來分析VCD檔案中的APB交易，計算 Read / Write Transaction 的數量
此程式必須能在linux上運行
若需要parser的辨識結果先自己捏一個
之後夥伴會把parser 整合進來

VCD檔範例大致如下:
$date
    May 20, 2025  08:30:04
$end
$version
    TOOL:	xmsim(64)	19.09-s010
$end
$timescale
    1 ps
$end

$scope module test $end

$scope module apb_if $end
$var parameter 32 !    ADDR_WIDTH $end
$var parameter 32 "    DATA_WIDTH $end
$var wire      1 #    clk  $end
$var wire      1 $    rst_n  $end
$var reg      32 %    paddr [31:0] $end
$var reg      32 &    pwdata [31:0] $end
$var reg       1 '    pwrite $end
$var reg       1 (    psel $end
$var reg       1 )    penable $end
$var reg       1 *    pready $end
$var reg      32 +    prdata [31:0] $end
$upscope $end

$upscope $end

$enddefinitions $end
$dumpvars
b100000 "
b100000 !
0#
0$
b0 %
b0 &
0'
0(
0)
0*
b0 +
$end
#5000
1#
#10000
0#
#15000



完整輸出大致如下(目前只要做計算 Read / Write Transaction 的數量就好):
Number of Read Transactions with no wait states: 2711
Number of Read Transactions with wait states: 0
Number of Write Transactions with no wait states: 9
Number of Write Transactions with wait states: 0
Average Read Cycle: 2.00 cycles
Average Write Cycle: 2.00 cycles
Bus Utilization: 6.24%
Number of Idle Cycles: 81753
Number of Completer: 1
CPU Elapsed Time: 0.87 ms

Number of Transactions with Timeout: 0
Number of Out-of-Range Accesses: 0
Number of Mirrored Transactions: 0
Number of Read-Write Overlap Errors: 0

Completer 1 PADDR Connections
a31: Correct
a30: Correct
a29: Correct
a28: Correct
a27: Correct
a26: Correct
a25: Correct
a24: Correct
a23: Correct
a22: Correct
a21: Correct
a20: Correct
a19: Correct
a18: Correct
a17: Correct
a16: Correct
a15: Correct
a14: Correct
a13: Correct
a12: Correct
a11: Correct
a10: Correct
a09: Correct
a08: Correct
a07: Correct
a06: Correct
a05: Correct
a04: Correct
a03: Correct
a02: Correct
a01: Correct
a00: Correct

Completer 1 PWDATA Connections
d31: Correct
d30: Correct
d29: Correct
d28: Correct
d27: Correct
d26: Correct
d25: Correct
d24: Correct
d23: Correct
d22: Correct
d21: Correct
d20: Correct
d19: Correct
d18: Correct
d17: Correct
d16: Correct
d15: Correct
d14: Correct
d13: Correct
d12: Correct
d11: Correct
d10: Correct
d09: Correct
d08: Correct
d07: Correct
d06: Correct
d05: Correct
d04: Correct
d03: Correct
d02: Correct
d01: Correct
d00: Correct
