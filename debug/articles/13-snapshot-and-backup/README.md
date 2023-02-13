# leveldb 快照与备份

作为持久化存储，数据库的快照功能是必要的，所以 leveldb 提供了 `db->GetSnapshot()` 等 API。借助 leveldb MVCC 优秀的实现，可以轻易实现 Snapshot 相关功能，提供数据库的一份只读视图。

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

## snapshot implement

snapshot 借助 sequence_number 进行标识，通过 sequence_number 的偏序关系来定位快照间的增量数据，即 version_edit。

## snapshot and backup

注意到，快照可以是够用的副本，而未必是完整的副本。    
存储完整副本的过程称为“备份”，通过解析快照实现全量备份。