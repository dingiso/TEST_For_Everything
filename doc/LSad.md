# Load & Store

design document for load and store pipeline 

### Load

| stage | operation                                                    |
| ----- | ------------------------------------------------------------ |
| 0     | 1. 计算虚拟地址<br />2. 发送虚拟地址 到 TLB<br />3. 发送虚拟地址 到 Dcache 矩形 Tag 索引 |
| 1     | 1. 收到 TLB 的物理地址<br />2. 发送 物理地址 到 Dcache Tag 比较 |
| 2     | 1. 收到 返回的数据结果<br />2. 对数据进行 裁剪<br />3. 发送出结果(存储到寄存器中) |



### Store

把 stq 和 sbuffer 整合到 sta 中

#### Sta

| stage | operation                                             |
| ----- | ----------------------------------------------------- |
| 0     | 1. 从保留站获得 data<br />2. 将 data 写入 store queue |

#### Std

| stage | operation                                                    |
| ----- | ------------------------------------------------------------ |
| 0     | 1. 计算虚拟地址<br />2. 初始化 stq <br />3. 发送虚拟地址给 TLB |
| 1     | 1. 接收 TLB 的物理地址<br />2. 把物理地址写入 stq            |
| 2     | 1. 将 stq 的数据写入 sbuffer                                 |
| 3     | 1. 设置指令状态， 通知 ROB 可以提交指令                      |

