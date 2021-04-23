// RocksDB

#ifdef USE_RDB
#include "common.h"
#include <rocksdb/db.h>
#include <rocksdb/options.h>

const string_view DBNAME("kvtest.rdb");

using namespace ROCKSDB_NAMESPACE;

static DB *db = nullptr;

bool db_open(void) {
  Options options;
  options.create_if_missing = true;
  // options.unordered_write = true
  return DB::Open(options, DBNAME.cbegin(), &db).ok();
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->Put(WriteOptions(), string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).ok();
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  string val;
  return (db->Get(ReadOptions(), string_view((const char *) &k, sizeof(k)), &val).ok() and (*((uint32_t *) val.data()) == v));
}

int RecordTry(const uint160_t &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  if (!cli(argc, argv))
    return 1;
  if (!db_open())
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  delete db;
  if (test_get or test_ask or test_try) {
    if (!db_open())
      ret_err("Cannot reopen db", 2);
    if (test_get)
      stage_get(RecordGet);
    if (test_ask)
      stage_ask(RecordGet);
    if (test_try)
      stage_try(RecordTry);
    delete db;
  }
  out_result();
  return 0;
}
#endif
