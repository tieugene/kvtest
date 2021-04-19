#include "common.h"
#include <kcpolydb.h>

kyotocabinet::PolyDB db;

bool db_open(void) {
  return db.open("kvtest.kch", kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE | kyotocabinet::PolyDB::OTRUNCATE);
}

bool record_add(const uint160_t &k, const uint32_t v) {
  return db.add((const char *) &k, sizeof(k), (const char *)&v, sizeof(v));
}

bool record_get(const uint160_t &k) {
  uint32_t v;
  return db.get((const char *) &k, sizeof(k), (char *)&v, sizeof(v)) == sizeof (v);
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, db_open, record_add, record_get);
}
