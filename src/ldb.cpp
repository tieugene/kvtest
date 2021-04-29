// LevelDB

#ifdef USE_LDB
#include "common.h"
#include <leveldb/db.h>

const string DBNAME("kvtest.ldb");

static leveldb::DB *db = nullptr;
static leveldb::WriteOptions writeOptions;
static leveldb::ReadOptions readOptions;

bool db_open(const string &name, bool create=false) {
  leveldb::Options options;
  options.create_if_missing = create;
  return leveldb::DB::Open(options, name, &db).ok();
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  if (verbose)
    cerr << "   Sync... ";
  time_start();
  delete db;
  auto t = time_stop();
  if (verbose)
    cerr << t << " ms" << endl;
  return true;
}

bool RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  if (db->Put(writeOptions, string((const char *) &k, sizeof(k)), string((const char *)&v, sizeof(v))).ok())
    return true;
  else
    throw Err_Cannot_Add;
}

bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  string val;
  auto s = db->Get(readOptions, string((const char *) &k, sizeof(k)), &val);
  if (s.ok()) {
    if (*((uint32_t *) val.data()) == v)
      return true;
    else
      throw Err_Unexpected_Value;
  }
  else if (s.IsNotFound())
    return false;
  else
    throw Err_Cannot_Get;
}

int RecordTry(const KEYTYPE_T &k, const uint32_t v) {
  auto got = RecordGet(k, v);
  //cerr << "Get:" << got << endl;
  return got ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  // TODO: db->sync();
  if (!cli(argc, argv))
    return 1;
  string name = dbname.empty() ? DBNAME : dbname;
  if (!db_open(name, true))
    return ret_err("Cannot create db", 1);
  stage_add(RecordAdd);
  db_sync();
  auto dbsize = d_size(name);
  if (test_get or test_ask or test_try) {
    if (!db_open(name))
      ret_err("Cannot reopen db", 2);
    if (test_get)
      stage_get(RecordGet);
    if (test_ask)
      stage_ask(RecordGet);
    if (test_try)
      stage_try(RecordTry);
    db_sync();
  }
  out_result(dbsize);
  return 0;
}
#endif
