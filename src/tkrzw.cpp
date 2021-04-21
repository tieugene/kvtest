// Tkrzw

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <tkrzw_dbm_poly.h>

const string DBNAME = "kvtest.tkh";

tkrzw::PolyDBM db;

bool DbOpen(void) {
  return db.Open(DBNAME, true, tkrzw::File::OPEN_TRUNCATE).IsOK();
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db.Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).IsOK();
}

bool RecordGet(const uint160_t &k) {
  return db.Get(string_view((const char *) &k, sizeof(k)), nullptr).IsOK();
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
  // old way
  /// return RecordGet(k) ? -1 : int(RecordAdd(k, v));
  // new way
  string old_val;
  auto s = db.Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v)), false, &old_val);
  return (s == tkrzw::Status::DUPLICATION_ERROR) ? -1 : int(s.IsOK());
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
