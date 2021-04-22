// BerkeleyDB

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <db_cxx.h>

const string_view DBNAME("kvtest.bdb");

static Db *db = nullptr;

bool DbOpen(void) {
  db = new Db(nullptr, 0);
  return db->open(nullptr, DBNAME.cbegin(), nullptr, DB_HASH, DB_CREATE|DB_TRUNCATE, 0644) == 0;
}

bool DbReOpen(void) {
  // db can be unopenable after close()
  // https://docs.oracle.com/cd/E17076_02/html/api_reference/C/dbclose.html
  return (
        (db)
        and (!db->close(0))
        and (delete db, true)
        and (db = new Db(nullptr, 0))
        and (!db->open(nullptr, DBNAME.cbegin(), nullptr, DB_HASH, 0, 0644))
        );
}

bool DbClose(void) {
  return ((db) and (!db->close(0)));
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val((void *) &v, sizeof(v));
  return db->put(nullptr, &key, &val, DB_NOOVERWRITE) == 0;
}

bool RecordGet(const uint160_t &k) {
  Dbt key((void *) &k, sizeof(k)), val;
  return db->get(nullptr, &key, &val, 0) == 0;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, DbReOpen, DbClose, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
