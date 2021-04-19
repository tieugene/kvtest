#include "common.h"
#include <leveldb/db.h>

leveldb::DB *db;

leveldb::Status status;
leveldb::WriteOptions writeOptions;
leveldb::ReadOptions readOptions;

bool db_open(void) {
  leveldb::Options options;

  options.create_if_missing = true;
  status = leveldb::DB::Open(options, "ldb", &db);
  return status.ok();
}

bool record_add(const uint160_t &k, const uint32_t v) {
  leveldb::Slice key((const char *) &k, sizeof(k)), val((const char *)&v, sizeof(v));
  status = db->Put(writeOptions, key, val);
  return status.ok();
}

bool record_get(const uint160_t &k) {
  leveldb::Slice key((const char *) &k, sizeof(k));
  string val;
  status =  db->Get(readOptions, key, &val);
  return status.ok();
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, db_open, record_add, record_get);
}
