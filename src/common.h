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
static uint64_t t1, t2, t3, t4, kops1, kops2, kops3, kops4;
static bool test_get = true, test_ask = true, test_try = true;

int ret_err(string_view msg, int err) {
  cerr << msg << endl;
  return err;
}

bool cli(int argc, char *argv[])
{
    /*
     * Process CLI
     * Returns: success
     */
  bool retvalue = false;

  if ((argc < 2) or (argc > 3))
    cerr << "Usage: " << argv[0] << " <log2(records) (0..31)> [log2(tests) (0..31, default=" << TESTS_QTY << ")]" << endl;
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
  return (retvalue and (buffer = new uint160_t[RECS_QTY]));
}

inline void rand_u160(uint160_t &dst)
{
  /* generates 20-byte random int
   * ~10M/s/GHz
   */
  for (auto i = 0; i < 5; i++)
    dst[i] = rand();
}

uint64_t curtime(void) {
  return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void stage_add(function<bool (const uint160_t &, const uint32_t)> func_recadd) {
  uint160_t k;

  cerr << "1. Add " << RECS_QTY << " recs ... ";
  auto created = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < RECS_QTY; i++) {
      rand_u160(k);
      buffer[i] = k;
      if (func_recadd(k, i))
         created++;
  }
  t1 = curtime() - T0;
  kops1 = t1 ? RECS_QTY/t1 : 0;
  cerr << created << " / " << t1 << " ms (" << kops1 << " Kops)" << endl;
}

void stage_get(function<bool (const uint160_t &, const uint32_t)> func_recget) {
  uint32_t v;

  cerr << "2. Get " << TESTS_QTY << " recs ... ";
  auto found = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < TESTS_QTY; i++) {
      v = rand() % RECS_QTY;
      if (func_recget(buffer[v], v))
         found++;
  }
  t2 = curtime() - T0;
  kops2 = t2 ? TESTS_QTY/t2 : 0;
  cerr << found << " / " << t2 << " ms (" << kops2 << " Kops)" << endl;
}

void stage_ask(function<bool (const uint160_t &, const uint32_t)> func_recget) {
  uint160_t k;
  uint32_t v;

  cerr << "3. Ask " << TESTS_QTY << " recs ... ";
  auto found = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < TESTS_QTY; i++) {
      if (i & 1) {
        v = rand() % RECS_QTY;
        k = buffer[v];
      } else {
        v = RECS_QTY + i;
        rand_u160(k);
      }
      if (func_recget(k, v))
         found++;
  }
  t3 = curtime() - T0;
  kops3 = t3 ? TESTS_QTY/t3 : 0;
  cerr << found << " / " << t3 << " ms (" << kops3 << " Kops)" << endl;
}

void stage_try(function<int (const uint160_t &, const uint32_t)> func_recgetadd) {
  uint160_t k;
  uint32_t v;

  cerr << "4. Try " << TESTS_QTY << " recs ... ";
  auto created = 0;
  auto found = 0;
  auto T0 = curtime();
  for (uint64_t i = 0; i < TESTS_QTY; i++) {
      if (i & 1) {
        v = rand() % RECS_QTY;
        k = buffer[v];
      } else {
        v = RECS_QTY + i;
        rand_u160(k);
      }
      auto r = func_recgetadd(k, v);    // -1 if found, 1 if added, 0 if not found nor added
      if (r) {
          if (r == 1)
              created++;
          else
              found++;
      }
  }
  t4 = curtime() - T0;
  kops4 = t4 ? TESTS_QTY/t3 : 0;
  cerr << found+created << " / " << t4 << " ms (" << kops4 << " Kops): " << found << " get, " << created << " add" << endl;
}

void out_result(void) {
  cout << "Time(ms)/Kops:\t"
    << t1 << "\t" << t2 << "\t" << t3 << "\t" << t4 << "\t"
    << kops1 << "\t" << kops2 << "\t" << kops3 << "\t" << kops4 << endl;
}

#endif // COMMON_H
