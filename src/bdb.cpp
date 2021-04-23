// BerkeleyDB

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <db_cxx.h>

const string_view DBNAME("kvtest.bdb");

static Db *db = nullptr;

bool db_open(void) {
  if (!db)
    db = new Db(nullptr, 0);
  return db->open(nullptr, DBNAME.cbegin(), nullptr, DB_HASH, DB_CREATE|DB_TRUNCATE, 0644) == 0;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val((void *) &v, sizeof(v));
  return db->put(nullptr, &key, &val, DB_NOOVERWRITE) == 0;
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val;
  return ((db->get(nullptr, &key, &val, 0) == 0) and (*((uint32_t *) val.get_data()) == v));
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
  db->sync(0);
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->sync(0);
  }
  db->close(0);
  out_result();
  return 0;
}
#endif
