// LMDB

/* Cost:
 * 1 record = key+value+8 bytes = 20+4+8 = 32 bytes/rec
 */

#ifdef USE_MDB
#include <filesystem>
#include "common.h"
#include <lmdb.h>

const string_view DBNAME = "kvtest.mdb";

static MDB_env *env;
static MDB_txn *txn;
static MDB_dbi db;

bool debug_msg(int rc, const char *msg)
{
  cerr << msg << "error : " << mdb_strerror(rc) << endl;
  return false;
}

bool DbOpen(void) {
  int rc;

  if (!filesystem::exists(DBNAME))
    if (!filesystem::create_directory(DBNAME))
      return false;
  if ((rc = mdb_env_create(&env)))
    return debug_msg(rc, "mdb_env_create");
  if ((rc = mdb_env_open(env, DBNAME.begin(), 0, 0644)))
    return debug_msg(rc, "mdb_env_open");
  if ((rc = mdb_txn_begin(env, nullptr, 0, &txn)))
    return debug_msg(rc, "mdb_txn_begin");
  if ((rc = mdb_dbi_open(txn, nullptr, 0, &db)))
    return debug_msg(rc, "mdb_dbi_open");
  if ((rc = mdb_txn_commit(txn)))
    return debug_msg(rc, "mdb_txn_commit");
  return true;
}

bool DbReOpen(void) {
  // dummy
  return true;
}

bool DbClose(void) {
  // dummy
  return true;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  int rc;
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  val.mv_size = sizeof(v);
  val.mv_data = (void *) &v;
  if ((rc = mdb_txn_begin(env, nullptr, 0, &txn)))
    return debug_msg(rc, "mdb_txn_begin");
  if ((rc = mdb_put(txn, db, &key, &val, MDB_NOOVERWRITE)))
    return debug_msg(rc, "mdb_put");
  if ((rc = mdb_txn_commit(txn)))
    return debug_msg(rc, "mdb_txn_commit");
  return true;
}

bool RecordGet(const uint160_t &k) {
  int rc;
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  bool retvalue = true;
  if ((rc = mdb_txn_begin(env, nullptr, 0, &txn)))
    return debug_msg(rc, "mdb_txn_begin");
  retvalue = (mdb_get(txn, db, &key, &val) == 0);
  if ((rc = mdb_txn_commit(txn)))
    retvalue = debug_msg(rc, "mdb_txn_commit");
  return retvalue;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, DbReOpen, DbClose, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
