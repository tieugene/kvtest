// RocksDB

#ifdef USE_RDB

#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include "common.h"

const filesystem::path DBNAME("kvtest.rdb");

using namespace ROCKSDB_NAMESPACE;

static DB *db = nullptr;

bool db_open(const filesystem::path &name, bool create=false) {
  // TODO: WAL (.concurrent_prepare, WriteOptions::sync, DBOptions::manual_wal_flush
  Options options;
  options.create_if_missing = create;
  if (TUNING) {
    options.unordered_write = true;
    options.max_open_files = -1;
    options.compression = CompressionType::kNoCompression;
  }
  return DB::Open(options, name, &db).ok();
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    std::cerr << "   Sync... ";
  time_start();
  delete db;
  auto t = time_stop();
  if (verbose)
    std::cerr << t << " ms" << endl;
  return true;
}

void RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  if (!db->Put(WriteOptions(), string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).ok())
    throw KVTError(Err_Cannot_Add);
}

bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  string val;
  auto s = db->Get(ReadOptions(), string((const char *) &k, sizeof(k)), &val);
  if (s.ok()) {
    if (*((uint32_t *) val.data()) == v)
      return true;
    else
      throw KVTError(Err_Unexpected_Value);
  }
  else if (s.IsNotFound())
    return false;
  else
    throw KVTError(Err_Cannot_Get);
}

bool RecordTry(const KEYTYPE_T &k, const uint32_t v) {
  return RecordGet(k, v) or (RecordAdd(k, v), false);
}

int main(int argc, char *argv[]) {
  if (!cli(argc, argv))
    return 1;
  auto name = dbname.empty() ? DBNAME : dbname;
  if (!db_open(name, true))
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  db_sync();
  auto dbsize = d_size(name);
  if (test_get or test_ask or test_try) {
    if (!db_open(name))
      ret_err("Cannot reopen db", 2);
    if (test_get)
      stage_get(RecordGet);
    if (test_ask)
      stage_ask(RecordGet);
    if (test_try)
      stage_try(RecordTry);
    db_sync();
  }
  out_result(dbsize);
  return 0;
}
#endif
