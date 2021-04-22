// Kyotocabinet

#ifdef USE_KC
#include "common.h"
#include <kcpolydb.h>

const string DBNAME = "kvtest.kch";

static kyotocabinet::PolyDB *db = nullptr;

bool DbOpen(void) {
  db = new kyotocabinet::PolyDB();
  return ((db) and (db->open(DBNAME, kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE | kyotocabinet::PolyDB::OTRUNCATE)));
}

bool DbReOpen(void) {
  return ((db) and (!db->close()) and (!db->open(DBNAME)));
}

bool DbClose(void) {
  return ((db) and (!db->close()));
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->add((const char *) &k, sizeof(k), (const char *)&v, sizeof(v));
}

bool RecordGet(const uint160_t &k) {
  uint32_t v;
  return db->get((const char *) &k, sizeof(k), (char *)&v, sizeof(v)) == sizeof (v);
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
  return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, DbReOpen, DbClose, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
