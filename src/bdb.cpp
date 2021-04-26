/*
 * kvtest. BerkeleyDB backend
 */

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <map>
#include <iterator>
#include <db_cxx.h>

const map<string, DBTYPE> exts = {  ///< filename extensions allowable
  {".bdh", DB_HASH},
  {".bdt", DB_BTREE}
};
//{".bdq", DB_QUEUE},
//{".bdr", DB_RECNO},
const string DBNAME("kvtest.bdh");  ///< default filename
const string help = "\
.bdh: DB_HASH\n\
.bdt: DB_BTREE\
";

static Db *db = nullptr;            ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @param type Database type
 * @return true on success
 */
bool db_open(const string_view name, const DBTYPE type) {
  // TODO: use DB_UNKNOWN on reopening to detect type
  if (!db)
    db = new Db(nullptr, 0);
  return db->open(nullptr, name.cbegin(), nullptr, type, DB_CREATE|DB_TRUNCATE, 0644) == 0;
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync ... ";
  time_start();
  if (!db->sync(0)) {
      cerr << "Cannot sync DB" << endl;
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
 * @return true on success
 */
bool RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(KEYTYPE_T)), val((void *) &v, sizeof(uint32_t));
  return db->put(nullptr, &key, &val, DB_NOOVERWRITE) == 0;
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected
 */
bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(KEYTYPE_T)), val;
  return ((db->get(nullptr, &key, &val, 0) == 0) and (*((uint32_t *) val.get_data()) == v));
}

/**
 * @brief Get a record or add new callback
 * @param k key to get (if exists) or add
 * @param v value to add or expected if key exists
 * @return -1 if key exists *and* value found equal to expected, 1 if key-value added as new, 0 if not found nor added
 */
int RecordTry(const KEYTYPE_T &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

/**
 * @brief Programm entry point
 * @param argc command line options number
 * @param argv command line options strings
 * @return 0 if OK
 */
int main(int argc, char *argv[]) {
  string name = DBNAME;
  DBTYPE type = DB_HASH;
  auto search = exts.end();

  if (!cli(argc, argv))
    return 1;
  if (!dbname.empty()) {
    auto l = dbname.length();
    if ((l >= 4) and ((search = exts.find(dbname.substr(l - 4))) != exts.end())) {
          name = dbname;
          type = search->second;
    } else
      return ret_err("Filename must be *.ext, where 'ext' can be:\n" + help, 2);
  }
  if (!db_open(name, type))
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  if (!db_sync())
    return 2;
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->sync(0);
  }
  db_sync();
  out_result();
  return 0;
}
#endif
