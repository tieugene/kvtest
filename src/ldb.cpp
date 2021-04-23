// LevelDB

#ifdef USE_LDB
#include "common.h"
#include <leveldb/db.h>

const string_view DBNAME("kvtest.ldb");

static leveldb::DB *db = nullptr;
static leveldb::Status status;
static leveldb::WriteOptions writeOptions;
static leveldb::ReadOptions readOptions;

bool DbOpen(void) {
  leveldb::Options options;

  options.create_if_missing = true;
  return leveldb::DB::Open(options, DBNAME.cbegin(), &db).ok();
}

bool DbReOpen(void) {
  leveldb::Options options;
  options.create_if_missing = false;
  delete db;
  return leveldb::DB::Open(options, DBNAME.cbegin(), &db).ok();
}

bool DbClose(void) {
  delete db;
  return true;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  leveldb::Slice key((const char *) &k, sizeof(k)), val((const char *)&v, sizeof(v));
  status = db->Put(writeOptions, key, val);
  return status.ok();
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  leveldb::Slice key((const char *) &k, sizeof(k));
  string val;
  return (db->Get(readOptions, key, &val).ok() and (*((uint32_t *) val.data()) == v));
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, DbReOpen, DbClose, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
