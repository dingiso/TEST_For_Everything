### Load Queue

![Memory Block](https://xiangshan-doc.readthedocs.io/zh_CN/latest/figs/memblock/nanhu-memblock.png)

* 还有 ROB 和 Dispatch 直接相连

### 输入/出


* 从 dispatch 处接收 4 条指令
* 从 load 流水线接收 2 条指令的结果, 以更新其内部的状态
* 从 dcache 接收 miss refill 结果, 更新 load queue 中全部等待此次 refill 的指令的状态
* 写回 2 条 miss 的 load 指令. 这些指令已经取得了 refill 的数据, 会与正常的访存流水线争用两个写回端口


### 参数：

LoadQueueSize : 80 - 长度 

### 数据结构：

LoadQueue : 循环队列

* 物理地址：paddr

* 数据： data

* 状态位： 当前表项的状态，因为不是互斥的，所以利用 vec 的形式

  | 状态位      | 说明                                                         |
  | ----------- | ------------------------------------------------------------ |
  | allocated   | 该项已经被 dispatch 分配                                     |
  | datavalid   | load 指令已经拿到所需的数据                                  |
  | writebacked | load 指令结果已经写回到寄存器堆并通知 ROB, RS                |
  | miss        | load 指令未命中 dcache, 正在等待 dcache refill               |
  | pending     | 该 load 指令访问 mmio 地址空间, 执行被推迟. 正在等待指令成为 ROB 中最后一条指令 |
  | released    | load 指令访问的 cacheline 已经被 dcache 释放 (release)       |
  | error       | load 指令在执行过程中检测到错误                              |

### 时序：

#### Load Stage 2

在这一阶段, dcache 和前递返回结果, 并将以下内容写入到 load queue 中:（loadIn）

**fromLsPipelineBundle**

- 关键控制信号 （tlbMiss,wlineflag,isSoftPrefeth,isFirstIssue)
- 物理地址 
- 前递结果 (forwardMask,forwardData)
- trigger 检查结果

此时会被更新的状态标志位包括: `datavalid`, `writebacked`, `miss`, `pending`, `released`

[MMIO](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/lsq/load_queue/#mmio-uncached-访存) 与正常指令对 load queue 的更新方式不同.

#### Load Stage 3

s3_dcache_require_replay : 

* dcache 请求从指令从保留站中重发
* 将 `miss` 和 `datavalid` flag 更新为 false. 

这标志着这条指令会从保留站重发, 而不是在 load queue 中等待 refill 将其唤醒. 这一操作与 load 指令流水线中 stage 3 的保留站反馈操作同步发生.

s3_delayed_load_error:

* 同s2，只是结果较晚

* 同时传给 load queue 和写回

#### load 指令的完成

load 指令的完成指 load 取得结果, 写回 rob 和 rf 的操作. 

* 分奇偶两部分进行, 
* 较老的指令优先被选择. 每周期至多选出两条指令被写回. 

load queue 选择写回的指令有两种: 

- 已经完成的 mmio load 指令
- 此前 miss, 现在已经从 dcache 取得 refill 回来的数据结果的 load 指令

### 逻辑：

#### Load Queue Enqueue 

load 指令进入 load queue 实际分两步完成: `enqPtr` 的提前分配和 load queue 的实际写入. 

提前分配的原因是 dispatch 模块距离 MemBlock 太远, 将 load queue 处产生的 `enqPtr` 送到 dispatch 做为 `lqIdx` 需要面临很长的延迟. 南湖架构在 dispatch 附近维护 `enqPtr` 的提前分配逻辑, 由提前分配逻辑负责提供指令的 `lqIdx`.

（没打算做）

#### Load Queue 实际写入

在 load queue 被实际写入时, load queue 本身的 `enqPtr` 会根据写入 load queue 的指令数量被更新. 出于时序考虑, load queue 只会在 load queue 中 `空项数 >= enq指令数` 的情况下接受 dispatch 分派的指令
