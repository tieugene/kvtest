/*
 * kvtest - Tkrzw backend
 */

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <set>
#include <tkrzw_dbm_poly.h>

const set<string> exts = {".tkh", ".tkt", ".tks"};  ///< filename extensions allowable
const string DBNAME("kvtest.tkh");                  ///< default filename
const string help = "\
.tkh: HashDBM (file hash)\n\
.tkt: TreeDBM (file tree)\n\
.tks: SkipDBM (file ...)\
";

static tkrzw::PolyDBM *db = nullptr;

/**
 * @brief Open/create DB
 * @param name Database filename
 * @return true on success
 */
bool db_open(const string &name) {
  if (!db)
    db = new tkrzw::PolyDBM();
  return ((db) and db->Open(name, true, tkrzw::File::OPEN_TRUNCATE).IsOK());
}

/**
 * @brief Add a record to DB callback
 * @param k key
 * @param v value
 * @return true on success
 */
bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db->Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))).IsOK();
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected
 */
bool RecordGet(const uint160_t &k, const uint32_t v) {
  string val;
  return (db->Get(string_view((const char *) &k, sizeof(k)), &val).IsOK() and (*((uint32_t *) val.data()) == v));
}

/**
 * @brief Add a new record or get it if exists callback
 * @param k key to add-or-get
 * @param v value to add or expected if key exists
 * @return -1 if key exists *and* value found equal to expected, 1 if kay-value added as new, 0 if not found nor added
 */
int RecordTry(const uint160_t &k, const uint32_t v) {
  // old way
  // return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
  // new way
  string val;
  auto s = db->Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v)), false, &val);
  return ((s == tkrzw::Status::DUPLICATION_ERROR) and (*((uint32_t *) val.data()) == v)) ? -1 : int(s.IsOK());
}

/**
 * @brief Programm entry point
 * @param argc command line options number
 * @param argv command line options strings
 * @return 0 if OK
 */
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
