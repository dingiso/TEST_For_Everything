# TileLink

TileLink是一种通用的总线协议，支持在芯片内部的各个模块之间进行通信。TileLink协议定义了三种常用的请求类型：Acquire（获取）、Probe（探查）和Release（释放）。这些请求类型通常用于管理内存的访问

## Tag

Meta : cache line 的 tag 和 一致性状态

### TileLink Operation

| Operation | Purpose                       |
| --------- | ----------------------------- |
| Acquire   | cache 然后提高权限            |
| Probe     | 获取访问权限                  |
| Release   | write-back cache 然后降低权限 |

![operation](https://pdf.cdn.readpaper.com/parsed/fetch_target/dc992bdfd6c596bc033d9b1e40c46a13_38_Figure_15_-39443381.png)

1. **Acquire** 请求：Acquire 请求用于请求某个数据块的独占访问权。当一个处理器需要修改一个共享的数据块时，它将发送一个 Acquire 请求，以获得该数据块的独占访问权。如果其他处理器已经拥有该数据块的访问权，它们将收到一个 Probe 请求，以通知它们释放该数据块的访问权。
2. **Probe** 请求：Probe 请求用于检查数据块的状态，并通知其他处理器更新数据块的状态。当一个处理器修改一个共享的数据块时，它将发送一个 Write Acquire 请求，以获得该数据块的独占访问权。如果其他处理器已经拥有该数据块的访问权，它们将收到一个 Probe 请求，以通知它们更新该数据块的状态。
3. **Release** 请求：Release 请求用于释放数据块的访问权。当一个处理器完成对一个共享的数据块的修改后，它将发送一个 Release 请求，以通知其他处理器该数据块现在可以被共享访问。如果其他处理器已经拥有该数据块的访问权，它们将收到一个 Probe 请求，以通知它们该数据块现在可以被共享访问。

### Probe

在计算机体系结构中，Cache Probe 请求是一种用于检查 Cache 中数据块状态的操作。Cache Probe 请求通常由多处理器系统中的一个处理器发送，用于检查其他处理器的 Cache 中是否存在某个数据块，以及该数据块的状态。

Cache Probe 请求的目的是使系统中的所有处理器都能够共享同一份数据，并保证数据的一致性。当一个处理器修改了一个共享的数据块时，需要向其他处理器发送 Cache Probe 请求，以通知它们更新该数据块的状态。

Cache Probe 请求通常分为两种类型：

1. **Read Probe**：用于检查其他处理器的 Cache 中是否存在某个数据块，以及该数据块的状态。如果其他处理器的 Cache 中存在该数据块，并且状态为共享或已修改，那么该数据块将被读取到当前处理器的 Cache 中。
2. **Write Probe**：用于向其他处理器发送数据块的修改请求，并将其状态标记为已修改。如果其他处理器的 Cache 中存在该数据块，并且状态为共享或已修改，那么该数据块将被更新为已修改状态，并将其从其他处理器的 Cache 中删除。

通过 Cache Probe 请求，处理器可以保证系统中的所有 Cache 中都保持数据的一致性，从而避免因数据不一致而导致的错误和异常。

Probe Queue 接收到来自 L2 的 Probe 请求后的处理流程如下:

* 分配一项空的 Probe Entry;
* 向 Main Pipe 发送 probe 请求, 由于时序考虑该请求会被延迟一拍;
* 等待 Main Pipe 返回应答;
* 释放 Probe Entry.

###  Miss 

- 在 Miss Queue 中分配一项空的 Miss Entry, 并在 Miss Entry 中记录相关信息;
- 根据所在的块是否有效, 判断是否需要替换, 
- 如果要替换则向 Main Pipe 发送 replace 请求;
- 发送 replace 请求的同时向 L2 发送 Acquire 请求
- refill 数据回填;
- 释放 Miss Entry.
