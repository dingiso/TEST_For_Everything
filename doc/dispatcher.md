## 可扩展结构

### 规则

1. dispatching 是整个模拟过程的顺序执行的最后一步
2. 需要有一个 dispatcher 来实现灵活的与多的EXU 进行对接，
3. dispatch 会选择最老的，符合要求的指令发送给 dispatcher
4. dispatcher 会获取指令并转发给 EXU， 如果不能接收，dispatch 会卡住并等待 dispatcher 来唤醒他(能接收指令的时候)

### 设计

1. Dispatch 会创建 dispatcher 并利用提供给 Mavis 的JSON ISA 文件处理指令中指给定的 TargetUnit来选择合适的Exu。

#### Dispatcher

dispatcher 是 dispatch 和 执行逻辑的中转

1. 负责处理执行模块中的 credit
2. 为 dispatch 提供流控制，在不能接收指令的时候负责将其放回 dispatch 中的队列
3. 若在取消分发后能接收指令时，会唤醒 dispatch

#### Execute

执行单元将被扩展以处理来自调度的指令。等 scoreboard 完成，这个类将处理执行的就绪指令和非就绪指令。这个类将从一个内部稀疏数组中选择最老的、就绪的指令(Sparta: : Array)  ，并将自己标记为“忙”(目前为止)。将来，可以添加管道。当指令完成时(基于 Mavis JSON 文件中的延迟信息) ，指令将从内部问题队列中删除，并返回给调度程序相应的 credits

EXU 单元是由 ExecutionFactory 利用配置文件生成的。

#### YAML 

配置文件中模块的架构定义

```
top.extension.core_extensions:
  execution_topology:
    [["alu", "4"].
     ["fpu", "2"],
     ["br",  "1"]]
```

那么具体的流程就是， Dispatch 会创建 3 个 dispatchers， 然后 ExecutionFactory 会创建 4 个执行模块来处理 ALU 指令， 2 个来处理 FPU 指令，一个来处理 BR 指令

