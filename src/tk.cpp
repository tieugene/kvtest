/*
 * kvtest. Tkrzw backend
 */

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <set>
#include <map>
#include <tkrzw_dbm_poly.h>

const set<string> exts = {".tkh", ".tkt", ".tks"};  ///< filename extensions allowable
const string DBNAME("kvtest.tkh");                  ///< default filename
const string help = "\
.tkh: HashDBM (file hash)\n\
.tkt: TreeDBM (file tree)\n\
.tks: SkipDBM (file ...)\
";

static tkrzw::PolyDBM *db = nullptr;    ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @return true on success
 */
bool db_open(const string &name) {
  const map<string, string> tuning_params = {{"offset_width", "5"}};  // {"file", "PositionalParallelFile"}
  if (!db)
    db = new tkrzw::PolyDBM();
  //return ((db) and db->Open(name, true, tkrzw::File::OPEN_TRUNCATE).IsOK());
  return ((db) and db->OpenAdvanced(name, true, tkrzw::File::OPEN_TRUNCATE, tuning_params).IsOK());
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync... ";
  time_start();
  if (!db->Synchronize(true).IsOK()) {
      cerr << Err_Cannot_Sync << endl;
      return false;
  }
  auto t = time_stop();
  if (verbose)
    cerr << t << " ms" << endl;
  return true;
}

/**
 * @brief Add a record to DB callback
 * @param k key
 * @param v value
 * @throw unknow Something wrong
 */
void RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  if (!db->Set(string_view((const char *) &k, sizeof(KEYTYPE_T)), string_view((const char *)&v, sizeof(uint32_t))).IsOK())
    throw Err_Cannot_Add;
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected, false if not found
 * @throw unknow Something wrong or value found != expected
 */
bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  string val;
  auto status = db->Get(string_view((const char *) &k, sizeof(KEYTYPE_T)), &val);
  if (status.IsOK()) {
    if (*((uint32_t *) val.data()) == v)
      return true;
    else
      throw Err_Unexpected_Value;
  }
  else if (status == tkrzw::Status::NOT_FOUND_ERROR)
    return false;
  else
    throw status.GetMessage();
}

/**
 * @brief Get a record or add new callback
 * @param k key to get (if exists) or add
 * @param v value to add or expected if key exists
 * @return true if found and equal to expected, false if added, exception on error
 */
bool RecordTry(const KEYTYPE_T &k, const uint32_t v) {
  // old way: return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
  string val;
  auto s = db->Set(string_view((const char *) &k, sizeof(KEYTYPE_T)), string_view((const char *)&v, sizeof(uint32_t)), false, &val);
  if (s.IsOK())
    return false;
  else if (s == tkrzw::Status::DUPLICATION_ERROR) {
    if (*((uint32_t *) val.data()) == v)
      return true;
    else
      throw Err_Unexpected_Value;
  }
  else
    throw s.GetMessage();
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
  if (!db_sync())
    return 2;
  auto dbsize = db->GetFileSizeSimple();
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db_sync();
  }
  db->Close();
  out_result(dbsize);
  return 0;
}
#endif
