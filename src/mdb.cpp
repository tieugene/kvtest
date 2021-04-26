// LMDB

/* Cost:
 * 1 record = key+value+8 bytes = 20+4+8 = 32 bytes/rec
 */

#ifdef USE_MDB
#include <filesystem>
#include "common.h"
#include <lmdb.h>

const string DBNAME = "kvtest.mdb";
const uint64_t MAXMAPSIZE = 0x100000000; // 4GB

static MDB_env *env;
static MDB_dbi db;
static MDB_txn *txn;
static uint64_t mapsizes[32] = {
  1, 1, 1, 1, 1, 257, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1
};

bool debug_msg(int rc, const char *msg)
{
  cerr << msg << " error : " << mdb_strerror(rc) << endl;
  return false;
}

bool tx_open(unsigned int flags = 0) {
  int rc;
  if ((rc = mdb_txn_begin(env, nullptr, flags, &txn)))
    return debug_msg(rc, "mdb_txn_begin");
  return true;
}

bool tx_close(void) {
  int rc;
  if ((rc = mdb_txn_commit(txn)))
    return debug_msg(rc, "mdb_txn_commit");
  return true;
}


bool db_open(const string &name, uint64_t dbsize) {
  int rc;

  if (!filesystem::exists(name))
    if (!filesystem::create_directory(name))
      return false;
  if ((rc = mdb_env_create(&env)))
    return debug_msg(rc, "mdb_env_create");
  if ((rc = mdb_env_set_mapsize(env, dbsize)))
    return debug_msg(rc, "mdb_env_set_mapsize");
  if ((rc = mdb_env_open(env, name.data(), 0, 0644)))
    return debug_msg(rc, "mdb_env_open");
  if (!tx_open())
    return false;
  if ((rc = mdb_dbi_open(txn, nullptr, MDB_CREATE, &db)))
    return debug_msg(rc, "mdb_dbi_open");
  if (!tx_close())
    return false;
  return true;
}

bool db_close() {
  return true;
}

/**
 * @brief Flush DB into disk
 * @return true if Success
 */
bool db_sync(void) {
  int rc;
  if (verbose)
    cerr << "   Sync ... ";
  time_start();
  if ((rc = mdb_env_sync(env, 1)))
    return (!debug_msg(rc, "mdb_sync"));
  auto t = time_stop();
  if (verbose)
    cerr << t << " ms" << endl;
  return true;
}

bool RecordAdd(const KEYTYPE_T &k, const uint32_t v) {
  int rc;
  MDB_val key, val;
  key.mv_size = sizeof(KEYTYPE_T);
  key.mv_data = (void *) &k;
  val.mv_size = sizeof(v);
  val.mv_data = (void *) &v;
  if ((rc = mdb_put(txn, db, &key, &val, MDB_NOOVERWRITE)))
    return debug_msg(rc, "mdb_put");
  return true;
}

bool RecordGet(const KEYTYPE_T &k, const uint32_t v) {
  MDB_val key, val;
  key.mv_size = sizeof(KEYTYPE_T);
  key.mv_data = (void *) &k;
  return ((mdb_get(txn, db, &key, &val) == 0) and (*((uint32_t *) val.mv_data) == v));
}

int RecordTry(const KEYTYPE_T &k, const uint32_t v) {
    return RecordGet(k, v) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  if (!cli(argc, argv))
    return 1;
  string name = dbname.empty() ? DBNAME : dbname;
  // ⇃ not works ⇂
  // auto db_rec_size = sizeof (KEYTYPE_T) + sizeof(uint32_t) + 8; // 36
  // uint64_t db_size = ((RECS_QTY * db_rec_size / 4080) + 1) * 4096;
  // 5 = err (1..12)
  if (!db_open(name, MAXMAPSIZE))
    return ret_err("Cannot create db", 1);
  // 1. add
  if (!tx_open())
    return 2;
  stage_add(RecordAdd);
  if (!tx_close())
    return 3;
  if (!db_sync())
    return 5;
  if (test_get) {
    if (!tx_open(MDB_RDONLY))
      return 6;
    stage_get(RecordGet);
    if (!tx_close())
      return 7;
  }
  if (test_ask) {
    if (!tx_open(MDB_RDONLY))
      return 8;
    stage_ask(RecordGet);
    if (!tx_close())
      return 9;
  }
  if (test_try) {
    if (!tx_open())
      return 10;
    stage_try(RecordTry);
    if (!tx_close())
      return 11;
    if (!db_sync())
      return 12;
  }
  out_result();
  return 0;
}
#endif
