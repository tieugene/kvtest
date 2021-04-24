// BerkeleyDB

#ifdef USE_BDB
#include "common.h"
#include <string_view>
#include <map>
#include <iterator>
#include <db_cxx.h>

const map<string, DBTYPE> exts = {
  {".bdh", DB_HASH},
  {".bdt", DB_BTREE}
};
//{".bdq", DB_QUEUE},
//{".bdr", DB_RECNO},
const string DBNAME("kvtest.bdh");
const string help = "\
.bdh: DB_HASH\n\
.bdt: DB_BTREE\
";

static Db *db = nullptr;

bool db_open(const string_view name, const DBTYPE type) {
  // TODO: use DB_UNKNOWN on reopening to detect type
  if (!db)
    db = new Db(nullptr, 0);
  return db->open(nullptr, name.cbegin(), nullptr, type, DB_CREATE|DB_TRUNCATE, 0644) == 0;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val((void *) &v, sizeof(v));
  return db->put(nullptr, &key, &val, DB_NOOVERWRITE) == 0;
}

bool RecordGet(const uint160_t &k, const uint32_t v) {
  Dbt key((void *) &k, sizeof(k)), val;
  return ((db->get(nullptr, &key, &val, 0) == 0) and (*((uint32_t *) val.get_data()) == v));
}

int RecordTry(const uint160_t &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

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
  db->sync(0);
  if (test_get)
    stage_get(RecordGet);
  if (test_ask)
    stage_ask(RecordGet);
  if (test_try) {
    stage_try(RecordTry);
    db->sync(0);
  }
  db->close(0);
  out_result();
  return 0;
}
#endif
