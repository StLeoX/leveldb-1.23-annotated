# leveldb 读取流程分析

leveldb 的典型应用场景是 `写多读少`，同时 Read Path 也有分析的必要。

## Read API

1. get

```cpp
leveldb::ReadOptions options;
std::string key1{"key1"}, value1;
db->Get(options, key1, &value1);
std::cout << key1 << ": " << value1 << std::endl;
```

如果不在内存中就需要去 SSTable 中查找，程序会走 TableCache::Get 来从 Meta Block 定位到 Data Block 的偏移，在走 Table::BlockReader 读出 Data Block，并完成解析、取出 value 值。

2. scan

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

```cpp
leveldb::ReadOptions options;
options.snapshot = db->GetSnapshot();
// ... apply some updates to the db ...
leveldb::Iterator* iter = db->NewIterator(options);
// ... using iter to view the previous state ...
delete iter;  // RAII
db->ReleaseSnapshot(options.snapshot);  // RAII
```

## Scan Cache

## Block Cache

Block Cache 缓存的是 Data Block，类似于传统的 Buffer Pool 部件，采用的是 LRU 策略。  
参数 `block_size` 决定了 Block Size，读操作中如果 `get`方式居多，该参数应该调大； `scan`方式居多，该参数应该调小；   
参数 `Cache *block_cache` 指定了堆上位置。

Block Cache 申请/释放 Block 是线程不安全的，所以在其 `Ref`、`Unref` 等引用计数操作前后需要进行并发控制。