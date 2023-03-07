# cache

主要分为数据结构和算法(接口) 

### ReplacementIF

LRU and MRU for way 

#### TreePLRUReplacement

对上述接口的实现, 利用数组实现基于二插树的PLRU算法

### AddrDecoderIF

对数据地址的解码操作

#### DefaultAddrDecoder

```cpp
/* DefaultAddrDecoder: decodes a 64-bit address.
 *
 * Assuming line_size==stride, the address is decoded as below
 *    +--------------------------+------+------+
 *    |tag                       |idx   |offset|
 *    *--------------------------+------+------+
 */
```

### BlockMemoryIF

作为整体内存看待的 read 和 write 接口

## 数据结构

#### BasicCacheItem (no data)-> LineData

```cpp
protected:
    uint32_t       set_idx_ = INVALID_VALUE_;
    uint32_t       way_num_ = INVALID_VALUE_;
    uint64_t       addr_ = 0;
    uint64_t       tag_  = 0;
    const AddrDecoderIF *addr_decoder_ = nullptr;
private:
    static const uint32_t INVALID_VALUE_ = 0xFFFFFFFF
```

#### BasicCacheSet

```c
const uint32_t          set_idx_;
const uint32_t          num_ways_;
ReplacementIF          *replacement_policy_;
std::vector<CacheItemT> ways_;
```

#### Cache

```c
const uint32_t                      num_sets_;
const uint32_t                      num_ways_;
std::vector<CacheSetT>  sets_;
```



### Coherency

MESI 作为 state 模式的

```cpp
//Coherency states
//-Coherency states (MESI) are not known or managed by cache library
bool isValid() const     { return valid_; }
bool isModified() const  { return modified_; }
bool isExclusive() const { return exclusive_; }
bool isShared() const    { return shared_; }
```

### Preload

底层使用 yaml-cpp 的树,节点对预取的数据进行包装


