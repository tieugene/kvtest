#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <array>
#include <functional>

using namespace std;

typedef array<uint32_t, 5> uint160_t;
static uint160_t *buffer;
static uint32_t RECS_QTY = 1 << 20;
static uint32_t TESTS_QTY = 1 << 20;

bool cli(int argc, char *argv[])
{
    /*
     * Process CLI
     * Returns: success
     */
  bool retvalue = false;

  if ((argc < 2) or (argc > 3))
    cerr << "Usage: " << argv[0] << " <log2(records) (0..31)> [log2(tests) (0..31, default 20)]" << endl;
  else {
    auto i = atoi(argv[1]);
    if ((i < 1) or (i > 31))
      cerr << "Bad log2(records): " << argv[1] << endl;
    else {
      RECS_QTY = 1 << i;
      if (argc == 3) {
        i = atoi(argv[2]);
        if ((i < 1) or (i > 31))
          cerr << "Bad log2(tests): " << argv[2] << endl;
        else {
          TESTS_QTY = 1 << i;
          retvalue = true;
        }
      } else
        retvalue = true;
    }
  }
  return retvalue;
}

inline void rand_u160(uint160_t &dst)
{
  /* generates 20-byte random int
   * ~10M/s/GHz
   */
  for (auto i = 0; i < 5; i++)
    dst[i] = rand();
}

int mainloop(
    int argc,
    char *argv[],
    function<bool (void)> func_dbopen,
    function<bool (void)> func_dbreopen,
    function<bool (void)> func_dbclose,
    function<bool (const uint160_t &, const uint32_t)> func_recadd,
    function<bool (const uint160_t &)> func_recget,
    function<int (const uint160_t &, const uint32_t)> func_recgetadd
    )
{
    /* Main testing function
     */
  uint32_t created, found;
  uint160_t k;

  if (!cli(argc, argv))
    return 1;
  if (!(buffer = new uint160_t[RECS_QTY]))
    return 2;
  srand(time(nullptr));
  // go
  if (!func_dbopen()) {
      cerr << "Cannot create db" << endl;
      return 3;
  }
  cerr << "Process " << RECS_QTY << " records:" << endl;
  // 1. Add samples
  cerr << "1. Add ... ";
  created = 0;
  auto T0 = time(nullptr);
  for (uint64_t i = 0; i < RECS_QTY; i++) {
      rand_u160(k);
      buffer[i] = k;
      if (func_recadd(buffer[i], i))
         created++;
  }
  auto t1 = time(nullptr) - T0;
  auto ops1 = t1 ? created/t1 : 0;
  cerr << created << " / " << t1 << " sec. (" << ops1 << " ops)" << endl;
  // 2. get
  if (!func_dbreopen()) {
      cerr << "Cannot reopen db #1" << endl;
      return 4;
  }
  cerr << "2. Get ... ";
  found = 0;
  T0 = time(nullptr);
  for (uint64_t i = 0; i < TESTS_QTY; i++)
      if (func_recget(buffer[rand() % RECS_QTY]))
         found++;
  auto t2 = time(nullptr) - T0;
  auto ops2 = t2 ? found/t2 : 0;
  cerr << found << " / " << t2 << " sec. (" << ops2 << " ops)" << endl;
  // 3. get-or-add
  cerr << "3. Try ... ";
  if (!func_dbreopen()) {
      cerr << "Cannot reopen db #2" << endl;
      return 5;
  }
  created = found = 0;
  T0 = time(nullptr);
  for (uint64_t i = 0; i < TESTS_QTY; i++) {
      if (i & 1)
        k = buffer[rand() % RECS_QTY];
      else
        rand_u160(k);
      auto r = func_recgetadd(k, i);    // -1 if found, 1 if added, 0 if not found nor added
      if (r) {
          if (r == 1)
              created++;
          else
              found++;
      }
  }
  auto t3 = time(nullptr) - T0;
  auto sum = found+created;
  auto ops3 = t3 ? sum/t3 : 0;
  cerr << sum << " / " << t3 << " sec. (" << ops3 << " ops): " << found << " get, " << created << " add" << endl;
  cout << "Time:\t" << t1 << "\t" << t2 << "\t" << t3 << endl;
  cout << "Kops':\t" << round(ops1/1000.0) << "\t" << round(ops2/1000.0) << "\t" << round(ops3/1000.0) << endl;
  func_dbclose();
  return 0;
}

#endif // COMMON_H
