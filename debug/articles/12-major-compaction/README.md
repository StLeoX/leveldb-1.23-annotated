# leveldb Major Compaction & Seek Compaction

承接 **10-minor-compaction**，Major Compaction 发生在运行时的后台，主要是将位于不同层级的 SSTable 进行合并。

## Major Compaction 的触发时机

在 `NeedsCompaction()` 方法中：

```cpp
bool NeedsCompaction() const {
    Version* v = current_;
    return (v->compaction_score_ >= 1) || (v->file_to_compact_ != NULL);
  }
```

1. L0 层文件数目太多，或 Li 层文件总体积过大，触发 compaction。

- 如果某一层级文件的个数太多（指的是 level0）。
- 如果某一层级的文件总大小太大，超过门限值，则设置 v->compaction_score 为一个大于 1 的数。
- 如果存在某个或者某几个层级的 score 大于等于 1，选择 score 最高的那个 level。

2. `SSTable Seeking` 次数太多，触发 compaction。

- 如果某个文件 seek 的次数太多。除了 level 0 以外，任何一个 level 的文件内部是有序的，文件之间也是有序的。
- 但是 level(n) 和 level(n+1) 中的两个文件的 key 可能存在重叠。正是因为这种重叠，查找某个 key 值的时候，level(n) 的查找未命中，所以接着去 level(n＋1) 查找。

## Major Compaction 的具体过程

## Major Compaction 的优化思路

> 部分思路来自于 `RocksDB` 的优化方案。

**通过优化 SSTable 组织**  
[Indexing SST Files for Better Lookup Performance](https://rocksdb.org/blog/2014/04/21/indexing-sst-files-for-better-lookup-performance.html)

## Seek Compaction

**Seek Compaction 是一种 Major Compaction，复用了处理函数。**  
Seek Compaction 的目的是提升读流程中 level 的命中率，关键在于减少相邻 level 的 SSTable 之间的 key 区间重叠。  
具体做法是，对于 seek 触发的 compaction, 哪个 SSTable 文件无效 seek 的次数到了阈值，就对那个文件发起 major compaction，减少 level 和 level ＋ 1 的重叠情况。
