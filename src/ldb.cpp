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

bool RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  return db->Put(writeOptions, string((const char *) &k, sizeof(k)), string((const char *)&v, sizeof(v))).ok();
}

bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  string val;
  return (db->Get(readOptions, string((const char *) &k, sizeof(k)), &val).ok() and (*((uint32_t *) val.data()) == v));
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
  delete db;
  if (test_get or test_ask or test_try) {
    if (!db_open(name))
      ret_err("Cannot reopen db", 2);
    if (test_get)
      stage_get(RecordGet);
    if (test_ask)
      stage_ask(RecordGet);
    if (test_try)
      stage_try(RecordTry);
    delete db;
  }
  out_result();
  return 0;
}
#endif
