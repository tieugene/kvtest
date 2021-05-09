/*
 * kvtest. BerkeleyDB backend
 */

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <map>
#include <iterator>
#include <db_cxx.h>

const filesystem::path DBNAME("kvtest.bdh");  ///< default filename
static Db *db = nullptr;                      ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @param type Database type
 * @return true on success
 */
void db_open(const filesystem::path name) {
  if (!db)
    db = new Db(nullptr, 0);
  if (!db)
    throw KVTError(Err_Cannot_New);
  if (TUNING) {
    auto cache = long(round(get_RAM()*0.75))/(1<<20); // 3.9GB=>2, 31GB=>23, 256GB=>188
    if (cache)
      db->set_cachesize(cache, 0, 0);
  }
  if (db->open(nullptr, name.c_str(), nullptr, DB_HASH, DB_CREATE|DB_TRUNCATE, 0644))
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
  if (db->sync(0)) {
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
 */
void RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(KEYTYPE_T)), val((void *) &v, sizeof(uint32_t));
  if (db->put(nullptr, &key, &val, DB_NOOVERWRITE))
    throw KVTError(Err_Cannot_Add);
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected
 */
bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(KEYTYPE_T)), val;
  auto s = db->get(nullptr, &key, &val, 0);
  if (s == 0) {
    if (*((uint32_t *) val.get_data()) == v)
      return true;
    else
      throw KVTError(Err_Unexpected_Value);
  }
  else if (s == DB_NOTFOUND)
    return false;
  else
    throw KVTError(Err_Cannot_Get);
}

/**
 * @brief Get a record or add new callback
 * @param k key to get (if exists) or add
 * @param v value to add or expected if key exists
 * @return true if found and equal to expected, false if added, exception on error
 */
bool RecordTry(const KEYTYPE_T &k, const uint32_t v) {
    return RecordGet(k, v) or (RecordAdd(k, v), false);
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
  db_open(name);
  stage_add(RecordAdd);
  if (!db_sync())
    return 2;
  auto dbsize = f_size(name);
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db_sync();
  }
  out_result(dbsize);
  return 0;
}
#endif
/* TODO:
  // DB_HASH_STAT stat;
  // if (!db->stat(nullptr, &stat, DB_FAST_STAT))
  //  dbsize = stat.hash_pagecnt * stat.hash_pagesize;  // 32766 * 6 <> 20362/40702/... * 4096
  // db->stat_print(DB_FAST_STAT);
*/
