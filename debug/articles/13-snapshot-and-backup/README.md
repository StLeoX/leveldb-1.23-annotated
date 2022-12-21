# leveldb 快照与备份

作为持久化存储，数据库的快照功能是必要的，所以 leveldb 提供了 `db->GetSnapshot()` 等 API。借助 leveldb MVCC 优秀的实现，可以轻易实现 Snapshot 相关功能。

## snapshot and transaction

leveldb 并未显式提供事务 API，但通过 snapshot 可以实现**可重复读**这一隔离级别的事务。  
具体而言，只需要先创建快照：

```cpp
auto before = db->GetSnapshot();
```

再去读该快照，并实施事务操作即可：

```cpp
leveldb::ReadOptions options;
options.snapshot = before;
leveldb::Iterator* iter = db->NewIterator(options);
```
