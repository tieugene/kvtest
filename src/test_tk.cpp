// Test Tkrzw

#ifdef USE_TK
#include <string_view>
#include <array>
#include <tkrzw_dbm_poly.h>

using namespace std;

typedef array<uint32_t, 6> KEYTYPE_T;   ///< 24-bytes uint
const string DBNAME = "kvtest.tkh";

void get_key(const uint32_t v, KEYTYPE_T &dst) {
  dst[0] = rand();
  dst[1] = v;
  dst[2] = ~v;
  dst[3] = rand();
  dst[4] = ~v ^ 0xFFFF0000;
  dst[5] = v ^ 0xFFFF;
}
int main(int argc, char *argv[]) {
  tkrzw::PolyDBM db;
  uint32_t reccount, added = 0;
  KEYTYPE_T k;

  if ((argc != 2) or (atoi(argv[1]) < 1)) {
      cerr << "Usage: test_tk <records>" << endl;
      return 0;
  }
  if (!db.Open(DBNAME, true, tkrzw::File::OPEN_TRUNCATE).IsOK()) {
      cerr << "Cannot open db>" << endl;
      return 0;
  }
  srand(time(nullptr));
  reccount = atoi(argv[1]);
  // start
  cerr << "Creating " << reccount << " records ... ";
  for (uint32_t v = 0; v < reccount; v++) {
    get_key(v, k);
    if (db.Set(string_view((const char *) &k, sizeof(KEYTYPE_T)), string_view((const char *)&v, sizeof(uint32_t))).IsOK())
      added++;
  }
  cerr << added << " OK." << endl;
  // end
  if (!db.Synchronize(true).IsOK())
    cerr << "Cannot sync db" << endl;
  db.Close();
  return 0;
}
#endif
