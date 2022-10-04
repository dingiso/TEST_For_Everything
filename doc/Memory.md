# Memory


## 总体架构

![Memory Block](https://xiangshan-doc.readthedocs.io/zh_CN/latest/figs/memblock/nanhu-memblock.png)

* 2 *[load 流水线](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/fu/load_pipeline/) + 2 *[sta 流水线](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/fu/store_pipeline/#Sta-Pipeline) + 2 *[std 流水线](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/fu/store_pipeline/#Std-Pipeline).
* queue 负责维护顺序信息

* load queue : load 指令在一级缓存中缺失时负责监听后续的重填结果并执行写回操作
* store queue: 指令提交之前暂存 store 的数据, 并为 store 向 load 的前递提供数据 
* committed store buffer : 合并写请求，接近满时统一写入dcache
* Data Cache : 读，写，重填端口 TileLink 总线协议
* MMU ： 虚拟地址翻译物理地址  [TLB](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/mmu/tlb/), [L2TLB](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/mmu/l2tlb/), [Repeater](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/mmu/mmu/#repeater), [PMP & PMA](https://xiangshan-doc.readthedocs.io/zh_CN/latest/memory/mmu/pmp_pma/) 

## Load Pipeline

Stage 

0. 计算地址, 读 TLB, 读 dcache tag
1. 读 dcache data
2. 获得读结果，选择并写回

## Store Pipeline

![Sta Pipeline](https://xiangshan-doc.readthedocs.io/zh_CN/latest/figs/memblock/store-pipeline.png)

香山采用了 store 地址与数据分离的执行方式. store 的地址与数据在就绪时可以分别从保留站中被发出, 进入后续的处理流程. 同一条 store 指令对应的数据/地址两个操作靠相同的 robIdx / sqIdx 联系在一起.

香山将 store addr 流水线分成4个流水级. 前两个流水级负责将 store 的控制信息和地址传递给 store queue, 后两个流水级负责等待访存依赖检查完成. 

香山(std) 流水线, 在保留站提供 store data 后, store data 流水线会立刻将 data 写入 store queue 的对应项中. 

stage

0. 计算地址, 读 TLB
1. addr 和其他控制信息写入 store queue, 开始违例检查
2. 违例检查
3. 违例检查, 允许 store 指令提交

std

0. 保留站给出 store data 并写入 store queue