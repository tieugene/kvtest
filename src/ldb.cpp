// LevelDB
#ifdef LEVELDB
#include "common.h"
#include <leveldb/db.h>

leveldb::DB *db;

leveldb::Status status;
leveldb::WriteOptions writeOptions;
leveldb::ReadOptions readOptions;

bool DbOpen(void) {
  leveldb::Options options;

  options.create_if_missing = true;
  status = leveldb::DB::Open(options, "ldb", &db);
  return status.ok();
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  leveldb::Slice key((const char *) &k, sizeof(k)), val((const char *)&v, sizeof(v));
  status = db->Put(writeOptions, key, val);
  return status.ok();
}

bool RecordGet(const uint160_t &k) {
  leveldb::Slice key((const char *) &k, sizeof(k));
  string val;
  status =  db->Get(readOptions, key, &val);
  return status.ok();
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    /*
     * Returns:
     * -1 - found
     * +1 - added
     *  0 - not found nor added
     */
    if (RecordGet(k))
        return -1;
    return int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
