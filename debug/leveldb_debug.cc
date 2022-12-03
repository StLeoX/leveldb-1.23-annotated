//
// leveldb debug enter point
// including：1.performance metric
//

#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <vector>

#include "leveldb/db.h"
#include "leveldb/filter_policy.h"

static const int base = 64;
static const char base64_charset[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* 整数转 Base64 */
std::string intToBase64(int n) {
  std::string result;
  while (n) {
    result.push_back(base64_charset[n % base]);
    n = n / base;
  }
  while (result.size() < 6) {
    result.push_back('0');
  }
  reverse(result.begin(), result.end());
  return result;
}

/** 顺序写入
 *
 * @param keyCount 键总数
 * @param init 初值
 * @param step 步长
 */
void putSequentially(leveldb::DB* db, const leveldb::WriteOptions& writeOptions,
                     int keyCount, int init, int step) {
  int d = init;
  while (keyCount-- > 0) {
    std::string key = std::to_string(d);
    std::string value = intToBase64(d);
    db->Put(writeOptions, key, value);
    d += step;
  }
}

/** 随机写入
 *
 * @param keyCount 键总数
 * @param start 下界
 * @param end 上界
 */
void putRandomly(leveldb::DB* db, const leveldb::WriteOptions& writeOptions,
                 int keyCount, int start, int end) {
  while (keyCount-- > 0) {
    // random on [start, end)
    int d = (std::rand() % (end - start)) + start;
    std::string key = std::to_string(d);
    std::string value = intToBase64(d);
    db->Put(writeOptions, key, value);
  }
}

void test_putSequentially() {
  leveldb::Options options;
  options.create_if_missing = true;
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);
  leveldb::WriteOptions writeOptions;
  leveldb::ReadOptions readOptions;

  int numThreads = 8;
  int total = 10000;

  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/ldb_seq", &db);

  // put sequentially
  std::vector<std::thread> threads(numThreads);
  for (int i = 0; i < numThreads; i++) {
    threads[i] = std::thread(putSequentially, db, writeOptions,
                             total / numThreads, i, numThreads);
  }
  for (auto& t : threads) {
    t.join();
  }

  // scan
  auto iter =
      std::make_shared<leveldb::Iterator*>(db->NewIterator(readOptions));
  // deb
}

void test_putRandomly() {
  leveldb::Options options;
  options.create_if_missing = true;
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);
  leveldb::WriteOptions writeOptions;
  leveldb::ReadOptions readOptions;

  int numThreads = 8;
  int total = 10000;

  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/ldb_ran", &db);

  // put randomly
  std::vector<std::thread> threads(numThreads);
  for (int i = 0; i < numThreads; i++) {
    threads[i] = std::thread(putSequentially, db, writeOptions,
                             total / numThreads, i, i + numThreads);
  }
  for (auto& t : threads) {
    t.join();
  }

  // scan
  auto iter =
      std::make_shared<leveldb::Iterator*>(db->NewIterator(readOptions));
  // deb
}

void test_compact() {
  // deb
}

int main(int argc, char* argv[]) {
  std::cout << "test_putSequentially" << std::endl;
  test_putSequentially();

  // std::cout << "test_putRandomly" << std::endl;

  return 0;
}
