// RocksDB

#ifdef USE_RDB
#include "common.h"
#include <rocksdb/db.h>
#include <rocksdb/options.h>

using namespace ROCKSDB_NAMESPACE;

DB* db;

bool DbOpen(void) {
  Options options;
  options.create_if_missing = true;
  return DB::Open(options, "kvtest.rdb", &db).ok();
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->Put(WriteOptions(), string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).ok();
}

bool RecordGet(const uint160_t &k) {
  std::string value;
   return db->Get(ReadOptions(), string_view((const char *) &k, sizeof(k)), &value).ok();
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
