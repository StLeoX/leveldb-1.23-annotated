# leveldb Major Compaction

承接 **10-minor-compaction**，Major Compaction 发生在运行时的后台，主要是将位于不同层级的 SSTable 进行合并。

## Major Compaction 的触发时机

## Major Compaction 的具体过程

## Major Compaction 的优化思路

思路部分来自于 `RocksDB` 的方案。