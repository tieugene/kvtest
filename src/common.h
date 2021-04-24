/**
 * kvtest - common things.
 */

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <array>
#include <functional>
#include <unistd.h>   // getopt

using namespace std;

typedef array<uint32_t, 6> KEYTYPE_T;   ///< key type (24-bytes uint)
static uint32_t *rainbow;               ///< random uint32 numbers
const int RAINBOW_SIZE = 0x10000;       ///< 64K
static uint32_t RECS_QTY;               ///< Records to create in DB
static uint32_t TESTS_QTY = 1 << 20;    ///< Records to test Get/Ask/Try
static bool verbose = false;            ///< programm verbosity
static string dbname;                   ///< database file/dir name
static bool test_get = true, test_ask = true, test_try = true;  ///< Stages to execute
static uint64_t t1, t2, t3, t4, kops1, kops2, kops3, kops4;     ///< Results: times (ms) and speeds (kilo-operations per second) for all stages

static string_view help_txt = "\
Usage: [options] log2(records) (0..31))\n\
Options:\n\
-h        - this help\n\
-f <path> - file/dir name of DB/dir\n\
-n n      - log2(number of tests), 1..31 (default=20=>1M)\n\
-x (s)    - exclude steps (g=Get/a=Ask/t=Try)\n\
-v        - verbose\
";

/**
 * @brief Print message to stderr and return errcode
 * @param msg Message to print
 * @param err Error code to return
 * @return Error code given
 */
int ret_err(const string_view &msg, const int err) {
  cerr << msg << endl;
  return err;
}

/**
 * @brief Process command line and reserve memory for testing keys
 * @param argc arguments number
 * @param argv arguments strings
 * @return true on siccess
 */
bool cli(int argc, char *argv[]) {
  int opt, i;

  while ((opt = getopt(argc, argv, "hvf:n:x:")) != -1) {
    switch (opt) {
      case 'h':
        cerr << help_txt << endl;
        return false;
      case 'v':
        verbose = true;
        break;
      case 'n':
        i = atoi(optarg);
        if ((i < 1) or (i > 31)) {
            cerr << "-n must be 1..31" << endl;
            return false;
        }
        TESTS_QTY = 1 << i;
        break;
      case 'f':   // name
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
  if (!(rainbow = new uint32_t[RAINBOW_SIZE]))
    return false;
  // fill randoms
  srand(time(nullptr));
  for (i = 0; i < RAINBOW_SIZE; i++)
    rainbow[i] = rand();
  return true;
}

/**
 * @brief Get pseud-random key depending on given value (~100+M/s/GHz)
 * @param v value that key based on
 * @param dst key storage
 */
void get_key(const uint32_t v, KEYTYPE_T &dst) {
  dst[0] = rainbow[v & 0xFFFF];
  dst[1] = rainbow[v >> 16];
  dst[2] = v;
  dst[3] = ~v;
  dst[4] = rainbow[(~v) >> 16];
  dst[5] = rainbow[(~v) & 0xFFFF];
  //dst[0] = dst[1] = dst[2] = dst[3] = dst[4] = dst[5] = v;
}

/**
 * @brief Get current unixtime
 * @return Current unixtime in milliseconds
 */
uint64_t curtime(void) {
  return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

/**
 * @brief Create DB and add RECS_QTY testing records in it
 * @param func_recadd Callback to add a record into DB
 */
void stage_add(function<bool (const KEYTYPE_T &, const uint32_t)> func_recadd) {
  KEYTYPE_T k;

  if (verbose)
    cerr << "1. Add " << RECS_QTY << " recs ... ";
  auto created = 0;
  auto T0 = curtime();
  for (uint32_t v = 0; v < RECS_QTY; v++) {
      get_key(v, k);
      if (func_recadd(k, v))
         created++;
  }
  t1 = curtime() - T0;
  kops1 = t1 ? RECS_QTY/t1 : 0;
  if (verbose)
    cerr << created << " / " << t1 << " ms (" << kops1 << " Kops)" << endl;
}

/**
 * @brief Test getting TESTS_QTY existing records from DB
 * @param func_recget Callback to get a record from DB
 */
void stage_get(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  KEYTYPE_T k;

  if (verbose)
    cerr << "2. Get " << TESTS_QTY << " recs ... ";
  auto found = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < TESTS_QTY; i++) {
      uint32_t v = rand() % RECS_QTY;
      get_key(v, k);
      if (func_recget(k, v))
         found++;
  }
  t2 = curtime() - T0;
  kops2 = t2 ? TESTS_QTY/t2 : 0;
  if (verbose)
    cerr << found << " / " << t2 << " ms (" << kops2 << " Kops)" << endl;
}

/**
 * @brief Test getting TESTS_QTY existing/random (50/50) records from DB
 * @param Callback to get a record from DB
 */
void stage_ask(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  KEYTYPE_T k;

  if (verbose)
    cerr << "3. Ask " << TESTS_QTY << " recs ... ";
  auto found = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < TESTS_QTY; i++) {
      uint32_t v = rand() % RECS_QTY;
      if (i & 1)
        v += RECS_QTY;
      get_key(v, k);
      if (func_recget(k, v))
         found++;
  }
  t3 = curtime() - T0;
  kops3 = t3 ? TESTS_QTY/t3 : 0;
  if (verbose)
    cerr << found << " / " << t3 << " ms (" << kops3 << " Kops)" << endl;
}

/**
 * @brief Test getting existing (50%) or adding not existing (50%) TESTS_QTY records in DB
 * @param Callback to get-or-add a record in DB
 */
void stage_try(function<int (const KEYTYPE_T &, const uint32_t)> func_rectry) {
  KEYTYPE_T k;

  if (verbose)
    cerr << "4. Try " << TESTS_QTY << " recs ... ";
  auto created = 0;
  auto found = 0;
  auto T0 = curtime();
  for (uint32_t i = 0; i < TESTS_QTY; i++) {
      uint32_t v = i;   // rand() % RECS_QTY - not works in macOS
      if (i & 1)
        v += RECS_QTY;
      get_key(v, k);
      auto r = func_rectry(k, v);    // -1 if found, 1 if added, 0 if not found nor added
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

/**
 * @brief Output test results to stdout
 */
void out_result(void) {
  cout << "Time(ms)/Kops:\t"
    << t1 << "\t" << t2 << "\t" << t3 << "\t" << t4 << "\t"
    << kops1 << "\t" << kops2 << "\t" << kops3 << "\t" << kops4 << endl;
}
#endif // COMMON_H
