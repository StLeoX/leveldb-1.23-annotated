## leveldb 源码分析

核心架构：
![](leveldb.png)

### 1. QuickStart

leveldb 本身是一个 Key-Value 存储引擎，类似于一个 C/C++ 库，并没有提供 `main` 函数。  
编译并调试自定义的 leveldb_debug 程序，含有 `main` 函数：

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make leveldb_debug 
gdb leveldb_debug  # 此时 leveldb_debug 就在 build 目录下，可直接进行 gdb 调试
```

编译并运行自定义的 leveldb_shell 程序，类似于 `redis-cli` 客户端：

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make leveldb_shell 
./leveldb_shell ${db_data_dir}  # 运行 shell
```

### 2. leveldb 核心流程梳理

1. [leveldb 概述与 LSM-Tree](/debug/articles/01-introduction/README.md)
2. [leveldb 中的常用数据结构](/debug/articles/02-data-structure/README.md)
3. [leveldb 中的 varint 与 Key 组成](/debug/articles/03-varint-and-key-format/README.md)
4. [leveldb Key-Value 写入流程分析](/debug/articles/04-write-process/README.md)
5. [leveldb 预写日志格式及其读写流程](/debug/articles/05-WAL/README.md)
6. [SSTable(01)—概览与 Data Block](/debug/articles/06-SSTable-data-block/README.md)
7. [SSTable(02)—Bloom Filter 与 Meta Block](/debug/articles/07-SSTable-meta-block/README.md)
8. [SSTable(03)—SSTable 之索引](/debug/articles/08-SSTable-index/README.md)
9. [SSTable(04)—Table Builder](/debug/articles/09-SSTable-table-builder/README.md)
10. [Compaction(01)—Minor Compaction](/debug/articles/10-minor-compaction/README.md)
11. [leveldb 版本控制概览](/debug/articles/11-version-control-overview/README.md)
12. [Compaction(02)—Major Compaction](/debug/articles/12-major-compaction/README.md)
13. [Snapshot 快照与备份](/debug/articles/13-snapshot-and-backup/README.md)
14. [leveldb Key-Value 读取流程分析](/debug/articles/14-read-path/README.md)

### 3. leveldb 源码之外

leveldb 作为专注的键值对持久化存储引擎，衍生出不少存储组件。这些组件往往提出来自己的一些优化和改进，可以由此反观 leveldb 的一些性能缺陷。  
这部分衍生内容不太会写进代码注释，主要记录在 `articles` 当中。
