// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_

#include <cstdint>
#include <vector>

#include "leveldb/slice.h"

namespace leveldb {

struct Options;

class BlockBuilder {
 public:
  explicit BlockBuilder(const Options* options);

  BlockBuilder(const BlockBuilder&) = delete;
  BlockBuilder& operator=(const BlockBuilder&) = delete;

  /* 清空 Block */
  void Reset();

  /* 向 Block 添加 Record（键值对） */
  void Add(const Slice& key, const Slice& value);

  /* 向 Block 添加 Restart Point 相关信息，完成构建 */
  Slice Finish();

  /* 返回 Block 的预估大小 */
  size_t CurrentSizeEstimate() const;

  /* 判断 buffer_ 是否为空 */
  bool empty() const { return buffer_.empty(); }

 private:
  const Options* options_;          /* Options 对象 */
  std::string buffer_;              /* User Space 缓冲区 */
  std::vector<uint32_t> restarts_;  /* Restart Points 数组，用于映射Point序号与偏移 */
  int counter_;                     /* Restart Points 循环计数器，用于计算Point序号 */
  bool finished_;                   /* 是否已经调用了 Finish() 方法 */
  std::string last_key_;            /* 最后添加的 Key */
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
