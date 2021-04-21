// LMDB

#ifdef USE_MDB
#include "common.h"
#include <lmdb.h>

const string_view DBNAME = "kvtest.mdb";

MDB_env *env;
MDB_txn *txn;
MDB_dbi db;

bool DbOpen(void) {
  int rc;

  rc = mdb_env_create(&env);
  if(rc) {
    cerr << "mdb_env_create error: " << mdb_strerror(rc) << endl;
    return false;
  }
  rc = mdb_env_open(env, DBNAME.begin(), 0, 0644);
  if (rc) {
    cerr << "mdb_env_open error: " << mdb_strerror(rc) << endl;
    return false;
  }
  rc = mdb_txn_begin(env, NULL, 0, &txn);
  if (rc) {
    cerr << "mdb_txn_begin error: " << mdb_strerror(rc) << endl;
    return false;
  }
  rc = mdb_dbi_open(txn, NULL, 0, &db);
  if (rc) {
      cerr << "mdb_dbi_open error: " << mdb_strerror(rc) << endl;
      return false;
  }
  rc = mdb_txn_commit(txn);
  if (rc) {
    cerr << "mdb_txn_commit: " << mdb_strerror(rc) << endl;
    return false;
  }
  return true;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  val.mv_size = sizeof(v);
  val.mv_data = (void *) &v;
  int rc = mdb_txn_begin(env, NULL, 0, &txn);
  if (rc) {
    cerr << "mdb_txn_begin error: " << mdb_strerror(rc) << endl;
    return false;
  }
  rc = mdb_put(txn, db, &key, &val, MDB_NOOVERWRITE);
  if (rc) {
    cerr << "mdb_put error: " << mdb_strerror(rc) << endl;
    return false;
  }
  rc = mdb_txn_commit(txn);
  if (rc) {
    cerr << "mdb_txn_commit: " << mdb_strerror(rc) << endl;
    return false;
  }
  return true;
}

bool RecordGet(const uint160_t &k) {
  MDB_val key, val;
  key.mv_size = sizeof(k);
  key.mv_data = (void *) &k;
  bool retvalue = true;
  int rc = mdb_txn_begin(env, NULL, 0, &txn);
  if (rc) {
    cerr << "mdb_txn_begin error: " << mdb_strerror(rc) << endl;
    return false;
  }
  retvalue = (mdb_get(txn, db, &key, &val) == 0);
  rc = mdb_txn_commit(txn);
  if (rc) {
    cerr << "mdb_txn_commit: " << mdb_strerror(rc) << endl;
    retvalue = false;
  }
  return retvalue;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    return RecordGet(k) ? -1 : int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
