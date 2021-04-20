// LMDB
#ifdef LMDB
#include "common.h"
#include <lmdb.h>

MDB_dbi db;

bool DbOpen(void) {
  return mdb_dbi_open(NULL, "kvtest.mdb", 0, &db);
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  val.mv_size = sizeof(v);
  val.mv_data = (void *) &v;
  return mdb_put(NULL, db, &key, &val, MDB_NOOVERWRITE) == 0;
}

bool RecordGet(const uint160_t &k) {
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  return mdb_get(NULL, db, &key, &val) == 0;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    if (RecordGet(k))
        return -1;
    return int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
