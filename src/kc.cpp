// Kyotocabinet

#ifdef USE_KC
#include "common.h"
#include <kcpolydb.h>

const string_view DBNAME("kvtest.kch");

static kyotocabinet::PolyDB *db = nullptr;

bool db_open(void) {
  if (!db)
    db = new kyotocabinet::PolyDB();
  return ((db) and (db->open(DBNAME.cbegin(), kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE | kyotocabinet::PolyDB::OTRUNCATE)));
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->add((const char *) &k, sizeof(k), (const char *)&v, sizeof(v));
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  uint32_t val;
  return ((db->get((const char *) &k, sizeof(k), (char *)&val, sizeof(val)) == sizeof (val)) and (val == v));
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
  if (!db->synchronize(true))
    return ret_err("Cannot sync db", 2);
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->synchronize(true);
  }
  db->close();
  out_result();
  return 0;
}
#endif
