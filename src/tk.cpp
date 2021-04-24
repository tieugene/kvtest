// Tkrzw

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <set>
#include <tkrzw_dbm_poly.h>

const set<string> exts = {".tkh", ".tkt", ".tks"};
const string DBNAME("kvtest.tkh");
const string help = "\
.tkh: HashDBM (file hash)\n\
.tkt: TreeDBM (file tree)\n\
.tks: SkipDBM (file ...)\
";

static tkrzw::PolyDBM *db = nullptr;

bool db_open(const string &name) {
  if (!db)
    db = new tkrzw::PolyDBM();
  return ((db) and db->Open(name, true, tkrzw::File::OPEN_TRUNCATE).IsOK());
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
  string name = DBNAME;
  if (!dbname.empty()) {
      auto l = dbname.length();
      if ((l < 4) or !exts.count(dbname.substr(l - 4)))
          return ret_err("Filename must be *.ext, where 'ext' can be:\n" + help, 2);
      name = dbname;
  }
  if (!db_open(name))
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  if (!db->Synchronize(true).IsOK())
    return ret_err("Cannot sync db", 2);
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->Synchronize(true);
  }
  db->Close();
  out_result();
  return 0;
}
#endif
