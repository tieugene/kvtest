#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <array>
#include <functional>

using namespace std;

typedef array<uint32_t, 5> uint160_t;
static uint160_t *buffer;

int cli(int argc, char *argv[])
{
    /*
     * Process CLI
     */
  int retvalue = 0;
  if (argc != 2)
    cerr << "Usage: " << argv[0] << " <pow_of_2>" << endl;
  else {
      retvalue = atoi(argv[1]);
      if (retvalue < 1)
        cerr << "Bad number: " << argv[1] << endl;
      else {
        if (retvalue > 32) {
            cerr << "You want to use " << (1l << retvalue) << " records. Only Duncan MacLeod has enough time to do that." << endl;
            retvalue = 0;
        }
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

bool fill_buffer(uint64_t n)
{
    /* Fill working buffer with random keys
     */
  buffer = new uint160_t[n];
  if (!buffer)
    return false;
  srand(time(nullptr));
  uint32_t *ptr = (uint32_t *) buffer;
  for (uint64_t i = 0; i < n*5; ptr++, i++)
      *ptr = rand();
  return true;
}

int mainloop(
    int argc,
    char *argv[],
    function<bool (void)> func_dbopen,
    function<bool (const uint160_t &, const uint32_t)> func_recadd,
    function<bool (const uint160_t &)> func_recget,
    function<int (const uint160_t &, const uint32_t)> func_recgetadd
    )
{
    /* Main testing function
     */
  uint64_t qty, created, found;
  uint160_t k;

  auto n = cli(argc, argv);
  if (n < 1)
    return 1;
  qty = 1l << n;
  if (!fill_buffer(qty))
    return 2;
  // go
  if (!func_dbopen()) {
      cerr << "Cannot create db" << endl;
      return 3;
  }
  cerr << "Process " << qty << " records:" << endl;
  // 1. Add samples
  cerr << "1. Add ....... ";
  created = 0;
  auto T0 = time(nullptr);
  for (uint64_t i = 0; i < qty; i++)
      if (func_recadd(buffer[i], i))
         created++;
  auto t1 = time(nullptr) - T0;
  cerr << created << " / " << t1 << " sec." << endl;
  // 2. get
  cerr << "2. Get ....... ";
  found = 0;
  T0 = time(nullptr);
  for (uint64_t i = 0; i < qty; i++)
      if (func_recget(buffer[rand() % qty]))
         found++;
  auto t2 = time(nullptr) - T0;
  cerr << found << " / " << t2 << " sec." << endl;
  // 3. get-or-add
  cerr << "3. Get|Add ... ";
  created = found = 0;
  T0 = time(nullptr);
  for (uint64_t i = 0; i < qty; i++) {
      if (i & 1)
        k = buffer[i];
      else
        rand_u160(k);
      auto r = func_recgetadd(k, i);
      if (r) {
          if (r == 1)
              created++;
          else
              found++;
      }
  }
  auto t3 = time(nullptr) - T0;
  cerr << (found+created) << " / " << t3 << " sec. (" << found << " get, " << created << " add)" << endl;
  cout << t1 << " " << t2 << " " << t3 << endl;
  return 0;
}

#endif // COMMON_H
