/*
 * kvtest. Kyotocabinet backend
 */

#ifdef USE_KC
#include "common.h"
#include <kcpolydb.h>

const set<string> exts = {".kch", ".kct", ".kcd", ".kcf"};  ///< filename extensions allowable
const string DBNAME("kvtest.kch");    ///< default filename
const string help = "\
.kch: HashDB (file hash)\n\
.kct: TreeDB (file tree)\n\
.kcd: DirDB (directory hash)\n\
.kcf: ForestDB (directory tree)\
";

static kyotocabinet::PolyDB *db = nullptr;    ///< DB handler

/**
 * @brief Open/create DB
 * @param name Database filename
 * @return true on success
 */
bool db_open(const string &name) {
  if (!db)
    db = new kyotocabinet::PolyDB();
  return ((db) and (db->open(name, kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE | kyotocabinet::PolyDB::OTRUNCATE)));
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync ... ";
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
 * @return true on success
 */
bool RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  return db->add((const char *) &k, sizeof(KEYTYPE_T), (const char *)&v, sizeof(uint32_t));
}

/**
 * @brief Get a record from DB callback
 * @param k key to search
 * @param v expected value if found
 * @return true if found *and* equal to expected
 */
bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  uint32_t val;
  return ((db->get((const char *) &k, sizeof(KEYTYPE_T), (char *)&val, sizeof(uint32_t)) == sizeof (uint32_t)) and (val == v));
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
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db_sync();
  }
  db->close();
  out_result();
  return 0;
}
#endif
