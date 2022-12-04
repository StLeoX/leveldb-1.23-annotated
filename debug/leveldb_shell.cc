#include <iostream>
#include <memory>
#include <vector>

#include "leveldb/db.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

void PrintHelp() {
  cout << "usage: leveldb_shell ${db_data_dir}" << endl;
  cout << "supported commands:\n"
          "- put ${key} ${value}: insert KV;\n"
          "- get ${key}: get KV by key;\n"
          "- del ${key}: delete KV by key;\n"
          "- scan \"${begin_key}\" \"${end_key}\": get KVs from begin_key to end_key;\n"
       << endl;
}

std::vector<string> Split(const string& s, char c = ' ') {
  if (s.empty()) return {};
  std::vector<string> res;
  string::size_type pos = s.find(c), prev = 0;
  while (pos != string::npos) {
    if (pos != prev) {
      res.emplace_back(begin(s) + prev, begin(s) + pos);
    }
    prev = pos + 1;
    pos = s.find(c, prev);
  }
  if (prev < s.size() && s[prev] != c) {
    res.emplace_back(begin(s) + prev, end(s));
  }
  return res;
}

leveldb::Status ParseScanParameters(const string& s, string* param) {
  if (s.size() < 2 || s.front() != s.back() ||
      s.front() != '"' && s.front() != '\'') {
    return leveldb::Status::InvalidArgument(s, "should be like \"key\"");
  }
  *param = s.substr(1, s.size() - 2);
  return leveldb::Status::OK();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    PrintHelp();
    exit(-1);
  }
  leveldb::DB* db_ptr;
  leveldb::Options opt;
  opt.create_if_missing = true;
  auto status = leveldb::DB::Open(opt, argv[1], &db_ptr);
  if (!status.ok()) {
    cout << status.ToString() << endl;
    exit(-1);
  }
  auto db = std::shared_ptr<leveldb::DB>(db_ptr);

  string command;
  cout << ">>> " << std::flush;
  while (getline(cin, command)) {
    auto commands = Split(command);
    if (commands.empty()) {
      goto PREFIX;
    }
    if (commands[0] == "put" && commands.size() == 3) {
      status = db->Put({}, commands[1], commands[2]);
      cout << status.ToString() << endl;
    } else if (commands[0] == "get" && commands.size() == 2) {
      string value;
      status = db->Get({}, commands[1], &value);
      if (!status.ok()) {
        cout << status.ToString() << endl;
      }
      cout << value << endl;
    } else if (commands[0] == "scan" && commands.size() == 3) {
      std::string start, end;
      auto iter = std::shared_ptr<leveldb::Iterator>(db->NewIterator({}));
      iter->Seek(start);
      while (iter->Valid() && iter->status().ok()) {
        status = ParseScanParameters(commands[1], &start);
        if (!status.ok()) {
          cout << status.ToString() << endl;
          break;
        }
        status = ParseScanParameters(commands[2], &end);
        if (!status.ok()) {
          cout << status.ToString() << endl;
          break;
        }
        if (!end.empty() && iter->key().compare(end) >= 0) {
          break;
        }
        cout << iter->key().ToString() << ":" << iter->value().ToString()
             << endl;
        iter->Next();
      }
      if (!iter->status().ok()) {
        cout << iter->status().ToString() << endl;
      }
    } else if (commands[0] == "del" && commands.size() == 2) {
      status = db->Delete({}, commands[1]);
      if (!status.ok()) {
        cout << status.ToString() << endl;
      }
    } else {
      cout << "invalid command" << endl;
      PrintHelp();
    }
  PREFIX:
    cout << ">>> " << std::flush;
  }

  return 0;
}