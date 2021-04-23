// Tkrzw

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <tkrzw_dbm_poly.h>

const string_view DBNAME("kvtest.tkh");

static tkrzw::PolyDBM *db = nullptr;

bool db_open(void) {
  if (!db)
    db = new tkrzw::PolyDBM();
  return ((db) and db->Open(DBNAME.cbegin(), true, tkrzw::File::OPEN_TRUNCATE).IsOK());
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).IsOK();
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  string val;
  return (db->Get(string_view((const char *) &k, sizeof(k)), &val).IsOK() and (*((uint32_t *) val.data()) == v));
}

int RecordTry(const uint160_t &k, const uint32_t v) {
  // old way
  /// return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
  // new way
  string val;
  auto s = db->Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v)), false, &val);
  return ((s == tkrzw::Status::DUPLICATION_ERROR) and (*((uint32_t *) val.data()) == v)) ? -1 : int(s.IsOK());
}

int main(int argc, char *argv[]) {
  if (!cli(argc, argv))
    return 1;
  if (!db_open())
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  if (!db->Synchronize(true).IsOK())
    return ret_err("Cannot sync db", 2);
  if (test_get)
    stage_get(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->Synchronize(true);
  }
  db->Close();
  out_result();
  return 0;
}
#endif
