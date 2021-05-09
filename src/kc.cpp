/*
 * kvtest. Kyotocabinet backend
 */

#ifdef USE_KC
#include "common.h"
#include <kchashdb.h>

const filesystem::path DBNAME("kvtest.kch");  ///< default filename
static kyotocabinet::HashDB *db = nullptr;    ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @return true on success
 */
void db_open(const filesystem::path &name, uint32_t recs) {
  if (!db)
    db = new kyotocabinet::HashDB();
  if (!db)
    throw Err_Cannot_New;
  if (TUNING) {
    if (!db->tune_buckets(recs))  // must be _before_ creating DB
      throw "Cannot tune DB";
    // db->tune_map(round(get_RAM()*0.5))) - danger
  }
  if (!db->open(name, kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE | kyotocabinet::HashDB::OTRUNCATE))
    throw Err_Cannot_Create;
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync... ";
  time_start();
  if (!db->synchronize(true)) {
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
 */
void RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  if (!db->add((const char *) &k, sizeof(KEYTYPE_T), (const char *)&v, sizeof(uint32_t)))
    throw Err_Cannot_Add;
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected
 */
bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  uint32_t val;
  auto s = db->get((const char *) &k, sizeof(KEYTYPE_T), (char *)&val, sizeof(uint32_t));
  if (s == sizeof (uint32_t)) {
    if (val == v)
      return true;
    else
      throw Err_Unexpected_Value;
  }
  else if (s == -1)
    return false;
  else
    throw Err_Cannot_Get;
}

/**
 * @brief Get a record or add new callback
 * @param k key to get (if exists) or add
 * @param v value to add or expected if key exists
 * @return -1 if key exists *and* value found equal to expected, 1 if key-value added as new, 0 if not found nor added
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
  db_open(name, RECS_QTY);
  stage_add(RecordAdd);
  if (!db_sync())
    return 2;
  auto dbsize = db->size();
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db_sync();
  }
  db->close();
  out_result(dbsize);
  return 0;
}
#endif
