// BerkeleyDB

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <db_cxx.h>

const string DBNAME = "kvtest.bdb";

Db *db;

bool DbOpen(void) {
  db = new Db(0, 0);
  return db->open(NULL, DBNAME.c_str(), NULL, DB_HASH, DB_CREATE|DB_TRUNCATE, 0) == 0;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val((void *) &v, sizeof(v));
  return db->put(NULL, &key, &val, DB_NOOVERWRITE) == 0;
}

bool RecordGet(const uint160_t &k) {
  Dbt key((void *) &k, sizeof(k)), val;
  return db->get(NULL, &key, &val, 0) == 0;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
