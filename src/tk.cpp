/*
 * kvtest. Tkrzw backend
 */

#ifdef USE_TK
#include "common.h"
#include <string_view>
#include <set>
#include <map>
#include <tkrzw_dbm_hash.h>

const filesystem::path DBNAME("kvtest.tkh");  ///< default filename
static tkrzw::HashDBM *db = nullptr;          ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @return true on success
 */
void db_open(const filesystem::path &name, uint32_t recs) {
  tkrzw::HashDBM::TuningParameters tuning_params;
  tuning_params.offset_width = 5;     // mandatory for large DB files: up to 2^(8*5)=1TB
  if (TUNING) {
    // TODO: find async write
    //tuning_params.align_pow = 2;      //align on 2^3=8 bytes
    tuning_params.num_buckets = recs; // buckets == records
    tuning_params.update_mode = tkrzw::HashDBM::UpdateMode::UPDATE_APPENDING;
    //tuning_params.lock_mem_buckets = true; // dangerous
  };
  if (!db)
    db = new tkrzw::HashDBM();
  if (!db)
    throw KVTError(Err_Cannot_New);
  if (!db->OpenAdvanced(name, true, tkrzw::File::OPEN_TRUNCATE, tuning_params).IsOK())
    throw KVTError(Err_Cannot_Create);
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync... ";
  time_start();
  //if (db->ShouldBeRebuiltSimple())
  //  db->Rebuild();
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
    throw KVTError(Err_Cannot_Add);
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
      throw KVTError(Err_Unexpected_Value);
  }
  else if (status == tkrzw::Status::NOT_FOUND_ERROR)
    return false;
  else
    throw KVTError(status.GetMessage());
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
      throw KVTError(Err_Unexpected_Value);
  }
  else
    throw KVTError(s.GetMessage());
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
  auto name = dbname.empty() ? DBNAME : dbname;
  db_open(name, RECS_QTY);
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

