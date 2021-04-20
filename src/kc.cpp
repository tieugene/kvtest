// Kyotocabinet
#ifdef KYOTOCABINET
#include "common.h"
#include <kcpolydb.h>

kyotocabinet::PolyDB db;

bool DbOpen(void) {
  return db.open("kvtest.kch", kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE | kyotocabinet::PolyDB::OTRUNCATE);
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db.add((const char *) &k, sizeof(k), (const char *)&v, sizeof(v));
}

bool RecordGet(const uint160_t &k) {
  uint32_t v;
  return db.get((const char *) &k, sizeof(k), (char *)&v, sizeof(v)) == sizeof (v);
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
