#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <array>
#include <functional>
#include <unistd.h>   // getopt

using namespace std;

typedef array<uint32_t, 5> uint160_t;
static uint160_t *buffer;
static uint32_t RECS_QTY = 1 << 20;
static uint32_t TESTS_QTY = 1 << 20;
static uint64_t t1, t2, t3, t4, kops1, kops2, kops3, kops4;
static bool verbose = false, test_get = true, test_ask = true, test_try = true;
static string dbname;

static string_view help_txt = "\
Usage: [options] log2(records) (0..31))\n\
Options:\n\
-h        - this help\n\
-n <path> - name of DB file/dir\n\
-t n      - log2(tests), 1..31 (default=20)\n\
-x (s)    - exclude steps (g=Get/a=Ask/t=Try)\n\
-v        - verbose\
";

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
  int opt, i;

  while ((opt = getopt(argc, argv, "hvn:t:x:")) != -1) {
    switch (opt) {
      case 'h':
        cerr << help_txt << endl;
        return false;
      case 'v':
        verbose = true;
        break;
      case 't':
        i = atoi(optarg);
        if ((i < 1) or (i > 31)) {
            cerr << "-t must be 1..31" << endl;
            return false;
        }
        TESTS_QTY = 1 << i;
        break;
      case 'n':   // name
        dbname = optarg;
        break;
      case 'x':   // exclude
        switch (*optarg) {
          case 'g':
            test_get = false;
            break;
          case 'a':
            test_ask = false;
            break;
          case 't':
            test_try = false;
            break;
          default:
            cerr << "-x must be g|a|t" << endl;
            return false;
        }
        break;
      case '?':   // can handle optopt
        cerr << help_txt << endl;
        return false;
    }
  }
  // cerr << "v=" << verbose << ", t=" << TESTS_QTY << ", n=" << dbname << ", xg=" << test_get << ", xa=" << test_ask << ", xt=" << test_try << endl;
  if ((argc - optind) != 1) {
    cerr << "Must be one mandatory argument" << endl;
    return false;
  }
  i = atoi(argv[optind]);
  if ((i < 1) or (i > 31)) {
    cerr << "Bad log2(records): " << argv[optind] << endl;
    return false;
  }
  RECS_QTY = 1 << i;
  return ((buffer = new uint160_t[RECS_QTY]));
}

inline void rand_u160(uint160_t &dst)
{
  /// generates 20-byte random int (~10M/s/GHz)
  for (auto i = 0; i < 5; i++)
    dst[i] = rand();
}

uint64_t curtime(void) {
  return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void stage_add(function<bool (const uint160_t &, const uint32_t)> func_recadd) {
  uint160_t k;

  if (verbose)
    cerr << "1. Add " << RECS_QTY << " recs ... ";
  srand(time(nullptr));
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
  if (verbose)
    cerr << created << " / " << t1 << " ms (" << kops1 << " Kops)" << endl;
}

void stage_get(function<bool (const uint160_t &, const uint32_t)> func_recget) {
  uint32_t v;

  if (verbose)
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
  if (verbose)
    cerr << found << " / " << t2 << " ms (" << kops2 << " Kops)" << endl;
}

void stage_ask(function<bool (const uint160_t &, const uint32_t)> func_recget) {
  uint160_t k;
  uint32_t v;

  if (verbose)
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
  if (verbose)
    cerr << found << " / " << t3 << " ms (" << kops3 << " Kops)" << endl;
}

void stage_try(function<int (const uint160_t &, const uint32_t)> func_recgetadd) {
  uint160_t k;
  uint32_t v;

  if (verbose)
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
  if (verbose)
    cerr << found+created << " / " << t4 << " ms (" << kops4 << " Kops): " << found << " get, " << created << " add" << endl;
}

void out_result(void) {
  cout << "Time(ms)/Kops:\t"
    << t1 << "\t" << t2 << "\t" << t3 << "\t" << t4 << "\t"
    << kops1 << "\t" << kops2 << "\t" << kops3 << "\t" << kops4 << endl;
}

#endif // COMMON_H
