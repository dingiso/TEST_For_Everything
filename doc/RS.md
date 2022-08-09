# 保留站-发射
## Reservation Station 

![后端设计图纸](https://xiangshan-doc.readthedocs.io/zh_CN/latest/figs/backend/backend.svg)

指令在发射阶段涉及到的主要包括入队、选择、读数据、出队等操作，同时保留站还需要负责对指令写回的监听以及对等待指令的唤醒等操作。香山目前采用了

#### 发射前读寄存器堆的设计方案
1. 数据的索引为保留站中的序号
2. 指令的操作数会在 RS 中多存储一份
3. 需要监听指令写回的数据

## 参数
1. numEntries: RS 中的表项
2. numRS: RS 的数目
3. numEnq: dpPorts/numRS
4. numDeq: 1 或 2 
   - (params.numDeq - maxRsDeq * i, maxRsDeq).min - maxRsDeq

## 数据结构

1. 指令状态 (Statue Array) 
   1. srcState : 源操作数状态
   2. Scheduled: 是否已经计划发射
   3. blocked  : 是否被阻塞
   4. [midState](#0) : 乘加指令的中间状态
   5. credit  : 该指令被调度之前所需的周期数

2. 指令存储 (Payload Array): 指令和地址

3. 数据存储 (Data Array)
   1. mask: 源操作数状态
   2. data: 源操作数值

   
## 算法
### SelectPolicy
 
使用二维数组age表示进入RS的前后关系，其中age(i)(i)表示第i个表项的指令是否还存在，而 age(i)(j)表示第 i 条指令在第 j 条指令后进入队列，每次会选择最老的指令
由于 AGE 算法的存在，当拍会选择 IssueWidth + 1 条指令

### 监听与唤醒

保留站需要监听指令的写回信号（在发射前读寄存器堆的设计中，还需要监听指令写回的数据），并相应地保存保留站中指令所需要的操作数。StatusArray 模块负责对写回总线进行监听，并判断哪些指令的操作数与之匹配。匹配信号及指令的写回数据会被送到 DataArray 的端口上，并实现写回数据的捕获。

## 时序 
<!--可以使用甘特图-->

#### TO(被选择)
* 入队并更新 status 和 payload
* StatusArray(可被唤醒的指令) —> SelectPolicy(就绪的指令) <=

#### T1(读数据)
* 寄存器堆数据读到 DataArray
* 就绪的指令 => 可供发射的指令

#### T2(出队)
* 完成握手并出队


<span id=0></span>

### 浮点乘加指令的发射策略优化

香山处理器的 FMA 单元支持乘加分离，可以同时处理浮点乘法和加法。因此，对于浮点乘加指令，我们在保留站实现了对他们的发射优化，当乘法的两个操作数就绪时，我们就允许指令进行发射，并将浮点乘法的中间结果写回保留站。当加法的第二个操作数就绪时，指令再次被发射，并完成全部运算。


## Reference
[Xiangshan-ISSUE](https://xiangshan-doc.readthedocs.io/zh_CN/latest/backend/issue/)

[BOOM-ISSUE-RS](https://docs.boom-core.org/en/latest/sections/issue-units.html)

[超标量处理器设计-姚永斌]

[Tomasulo算法](https://zhuanlan.zhihu.com/p/499978902)
