// RocksDB

#ifdef USE_RDB
#include "common.h"
#include <rocksdb/db.h>
#include <rocksdb/options.h>

const string_view DBNAME("kvtest.rdb");

using namespace ROCKSDB_NAMESPACE;

static DB *db = nullptr;

bool DbOpen(void) {
  Options options;
  options.create_if_missing = true;
  return DB::Open(options, DBNAME.cbegin(), &db).ok();
}

bool DbReOpen(void) {
  Options options;
  options.create_if_missing = false;
  delete db;
  return DB::Open(options, DBNAME.cbegin(), &db).ok();
}

bool DbClose(void) {
  delete db;
  return true;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->Put(WriteOptions(), string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).ok();
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  string val;
  return (db->Get(ReadOptions(), string_view((const char *) &k, sizeof(k)), &val).ok() and (*((uint32_t *) val.data()) == v));
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, DbReOpen, DbClose, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
