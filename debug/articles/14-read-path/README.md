# leveldb 读取流程分析

> [ref: LevelDB 设计与实现 - 缓存机制](https://zhuanlan.zhihu.com/p/51573464)

leveldb 的典型应用场景是 `写多读少`，同时 Read Path 也有分析的必要。

## Read API

1. get

get usecase:

```cpp
leveldb::ReadOptions options;
std::string key1{"key1"}, value1;
db->Get(options, key1, &value1);
std::cout << key1 << ": " << value1 << std::endl;
```

如果不在内存中就需要去 SSTable 中查找，程序会走 TableCache::Get 来从 Meta Block 定位到 Data Block 的偏移，在走 Table::BlockReader 读出 Data Block，并完成解析、取出 value 值。

2. scan

scan usecase:

```cpp
leveldb::ReadOptions options;
leveldb::Iterator* it = db->NewIterator(options);
for (it->SeekToFirst(); it->Valid(); it->Next()) {
  cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
}
assert(it->status().ok());
delete it;
```

3. snapshot

snapshot usecase:

```cpp
leveldb::ReadOptions options;
options.snapshot = db->GetSnapshot();
// ... apply some updates to the db ...
leveldb::Iterator* iter = db->NewIterator(options);
// ... using iter to view the previous state ...
delete iter;  // RAII
db->ReleaseSnapshot(options.snapshot);  // RAII
```

## Read path

查找流程：

1. `Memtable::Get()`先在 memtable 中查找；
2. `Memtable::Get()`如果在 memtable 中未找到，并且存在 immutable memtable，就在 immutable memtable 中查找；
3. `VersionSet::Get()`如果仍未找到，在 sstable 中查找。从 L0 开始，每个 level 上依次查找。

level 上的查找细节：

- 首先找出 level 上可能包含 key 的 sstable，FileMetaData 结构体内包含每个 sstable 的 key 范围。
- L0 的查找：只能顺序遍历每个 file，因为 L0 层的 sstable 文件之间可能存在重叠的 key。即在 L0 层可能找到多个 sstable。
- 非 L0 的查找：对 file 做二分查找，即可定位到 level 中可能包含 key 的 sstable。非 L0 上 sstable 之间 key 不会重叠，所以最多找到一个 sstable。
- 如果该 level 上没有找到可能的 sstable，跳过。
- 否则，对要进行查找的 sstable 获得其 Iterator，通过 seek() 检查删除标志（**称为 `SSTable Seeking`**）：
    - kTypeValue: 数据存在，返回对应的 value。
    - kTypeDeletion: 数据不存在，返回 data not exist。

## Table Cache

![](./images/table_cache.png)

每项由两部分组成：

- 指向打开的 SSTable 文件的文件指针；
- 指向内存中这个 SSTable 文件对应的 Table 结构指针；

通过这两个指针，就可以在内存级别重新打开 SSTable 文件、读到 Table 结构。

## Block Cache

![](./images/block_cache.png)

Block Cache 缓存的是 Data Block 键值对。它的实现类似于数据库中常见的 Buffer Pool 组件，采用的是 LRU 策略。   
Block Cache 部分参数可配置：

- 参数 `block_size` 决定了 Block Size，读操作中如果 `get`方式居多，该参数应该调大； `scan`方式居多，该参数应该调小；
- 参数 `Cache *block_cache` 指定了堆上位置。

Block Cache 申请/释放 Block 是线程不安全的，所以在其 `Ref`、`Unref` 等引用计数操作前后需要进行并发控制。

Block Cache LRU 实现细节：LRUCache 有两条链表，一条是 in_use 表示正在使用的 Cache，另一条是 lru_ 表示不经常使用的。数据项先从 in_use 淘汰到 lru_，再淘汰出去。