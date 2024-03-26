### PPP-B2b编解码

1. CRC算法生成多项式
$$g(x) = x^{24} + x^{23} + x^{18} + x^{17} + x^{14} + x^{11} + x^{10} + x^{7} + x^{6} + x^{5} + x^{4} + x^{3} + x + 1$$  
 
[CRC验证链接](http://www.ip33.com/crc.html)
 

##### 问题

1. URAL算法$\text{URA}[mm] \leq 3 ^ {\text{URA} _\text{CLASS}} \hspace{0.3em} \left(1 + 0.25 \times \text{URA} _{\text{VALUE}} \right) - 1$
计算结果和文档中（17页）不一致  

---
#### 编码类型对照表(需要更正)

|| BDS || GPS || GLONASS || Galileo||
|---|---|---|---|---|---|---|---|---|
|0| B1I |1I| L1 C/A |NAN| G1 C/A |1C| 预留|Reserved|
|1| B1C (D)|1C| L1 P |NAN| G1 P |1P| E1 B|1B|
|2| B1C (P)|1W| 预留 |Reserved| G2 C/A |2C| E1 C|1C|
|3| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
|4| B2a (D)|NAN| L1C (P)|1C| 预留 |Reserved| E5a Q|5aQ|
|5| B2a (P)|NAN| L1C (D+P)|NAN| 预留 |Reserved| E5a I|5aI|
|6| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
|7| B2b-I |2I| L2C (L)|2L| 预留 |Reserved| E5b I|5bI|
|8| B2b-Q |NAN| L2C (M+L)|NAN| 预留 |Reserved| E5b Q|5bQ|
|9| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
|10| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
|11| 预留 |Reserved| L5 I |5I| 预留 |Reserved| E6 C|6C|
|12| B3 I |3I| L5 Q |5Q| 预留 |Reserved| 预留|Reserved|
|13| 预留 |Reserved| L5 I+Q |NAN| 预留 |Reserved| 预留|Reserved|
|14| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
|15| 预留 |Reserved| 预留 |Reserved| 预留 |Reserved| 预留|Reserved|
