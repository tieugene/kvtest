/**
 * kvtest - common things.
 */

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <array>
#include <filesystem>
#include <functional>
#include <cmath>     // round()
#include <chrono>
#include <thread>
#include <unistd.h>   // getopt()

using namespace std;

// internal things
typedef array<uint32_t, 6> KEYTYPE_T;   ///< key type (24-bytes uint)
static uint32_t *rainbow;               ///< random uint32 numbers
const int RAINBOW_SIZE = 0x10000;       ///< 64K
// CLI options
static uint8_t RECS_POW;                ///< log2(Records to create)
static uint32_t RECS_QTY;               ///< Records to create
static uint8_t TEST_DELAY = 5;          ///< delay of each test, sec
static bool verbose = false;            ///< programm verbosity
static string dbname;                   ///< database file/dir name
// internal system-wide variables
static bool test_get = true, test_ask = true, test_try = true;  ///< Stages to execute
static uint32_t t1, ops1, ops2, ops3, ops4;     ///< Results: times (ms) and speeds (kilo-operations per second) for all stages
static bool can_play = false;
static chrono::time_point<chrono::steady_clock> T0;

static string_view help_txt = "\
Usage: [options] <log2(records) (0..31)>\n\
Options:\n\
-h        - this help\n\
-f <path> - file/dir name of DB\n\
-t n      - duration of each test, sec (1..255, default=5)\n\
-x s      - exclude step[s] (g=Get/a=Ask/t=Try)\n\
-v        - verbose on\
";

/**
 * @brief Process command line and reserve memory for testing keys
 * @param argc arguments number
 * @param argv arguments strings
 * @return true on siccess
 */
bool cli(int argc, char *argv[]) {
  int opt, i;

  while ((opt = getopt(argc, argv, "hf:t:x:v")) != -1) {
    switch (opt) {
      case 'f':   // name
        dbname = optarg;
        break;
      case 't':
        i = atoi(optarg);
        if ((i < 1) or (i > 255)) {
            cerr << "-t must be 1..255" << endl;
            return false;
        }
        TEST_DELAY = i;
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
      case 'v':
        verbose = true;
        break;
      case 'h':
      case '?':   // can handle optopt
        cerr << help_txt << endl;
        return false;
    }
  }
  if ((argc - optind) != 1) {
    cerr << "Must be one mandatory argument" << endl;
    return false;
  }
  i = atoi(argv[optind]);
  if ((i < 1) or (i > 31)) {
    cerr << "Bad log2(records): " << argv[optind] << endl;
    return false;
  }
  RECS_POW = i;
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
 * @brief Set can_play flag and reset it after 'sec' seconds
 * @param delay Delay for reseting can_play
 */
void lets_play(u_int8_t delay) {
  can_play = true;
  thread([=]() {
       this_thread::sleep_for(chrono::seconds(delay));
       can_play = false;
     }).detach();
}

/**
 * @brief Start timer
 */
void time_start(void) {
  T0 = std::chrono::steady_clock::now();
}

/**
 * @brief Get duration from time_start()
 * @return Milliseconds passed from time_start()
 */
uint32_t time_stop(void) {
  return chrono::duration<float, milli>(chrono::steady_clock::now() - T0).count();
}

/**
 * @brief Convert Kops into string
 * @param ops Operations per seconds
 * @return In representation of Kops or fixed point if Kops < 1
 */
const string opsKops(uint32_t ops) {
  if (ops == 0)
    return "0";
  else
    return (ops >= 1000) ? to_string(ops/1000) : to_string(ops/1000.0).substr(0, 5);
}

uint64_t f_size(string_view path) {
  return filesystem::file_size(path);
}

uint64_t d_size(string_view path) {
  uint64_t retvalue = 0;
  for (filesystem::directory_entry const& entry : filesystem::directory_iterator(path))
    if (entry.is_regular_file())
      retvalue += entry.file_size();
  return retvalue;
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
 * @brief Create DB and add RECS_QTY testing records in it
 * @param func_recadd Callback to add a record into DB
 */
void stage_add(function<bool (const KEYTYPE_T &, const uint32_t)> func_recadd) {
  uint32_t created = 0;
  KEYTYPE_T k;

  if (verbose)
    cerr << "Playing " << RECS_QTY << " records:" << endl
         << "1. Add ... ";
  time_start();
  for (uint32_t v = 0; v < RECS_QTY; v++) {
      get_key(v, k);
      if (func_recadd(k, v))
         created++;
  }
  t1 = time_stop();
  ops1 = t1 >= 0 ? RECS_QTY/t1 : 0;
  if (verbose)
    cerr << created << "/" << RECS_QTY << " @ " << t1 << " ms (" << opsKops(ops1*1000) << " Kops)" << endl;
}

/**
 * @brief Test getting TESTS_QTY existing records from DB
 * @param func_recget Callback to get a record from DB
 */
void stage_get(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  uint32_t all = 0, found = 0;
  KEYTYPE_T k;

  if (verbose)
    cerr << "2. Get ... ";
  lets_play(TEST_DELAY);
  while (can_play) {
    uint32_t v = rand() % RECS_QTY;
    get_key(v, k);
    if (func_recget(k, v))
       found++;
    all++;
  }
  ops2 = all/TEST_DELAY;
  if (verbose)
    cerr << found << "/" << all << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops2) << " Kops)" << endl;
}

/**
 * @brief Test getting TESTS_QTY existing/random (50/50) records from DB
 * @param Callback to get a record from DB
 */
void stage_ask(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  uint32_t all = 0, found = 0, not_recs_qty = ~RECS_QTY;
  KEYTYPE_T k;

  if (verbose)
    cerr << "3. Ask ... ";
  lets_play(TEST_DELAY);
  while (can_play) {
    uint32_t r = rand();
    uint32_t v = (all & 1) ? r % RECS_QTY : (r % not_recs_qty) + RECS_QTY;
    get_key(v, k);
    if (func_recget(k, v))
       found++;
    all++;
  }
  ops3 = all/TEST_DELAY;
  if (verbose)
    cerr << found << "/" << all << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops3) << " Kops), " << round(found*100/all) << "% found" << endl;
}

/**
 * @brief Test getting existing (50%) or adding not existing (50%) TESTS_QTY records in DB
 * @param Callback to get-or-add a record in DB
 */
void stage_try(function<int (const KEYTYPE_T &, const uint32_t)> func_rectry) {
  // FIXME: don't try to find 'unknown' keys that already added
  uint32_t all = 0, created = 0, found = 0, recs_qty = RECS_QTY;
  KEYTYPE_T k;

  if (verbose)
    cerr << "4. Try ... ";
  lets_play(TEST_DELAY);
  while (can_play) {
    uint32_t v = (all & 1) ? rand() % recs_qty : recs_qty;
    get_key(v, k);
    auto r = func_rectry(k, v);    // -1 if found, 1 if added, 0 if not found nor added
    if (r) {
      if (r == 1) {
        created++;
        recs_qty++;
      } else
        found++;
    }
    all++;
  }
  ops4 = all/TEST_DELAY;
  if (verbose)
    cerr << found+created << "/" << all << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops4) << " Kops): " << found << " (" << round(found*100/all) << "%) get, " << created << " add" << endl;
}

/**
 * @brief Output test results to stdout
 */
void out_result(uint64_t dbsize=0) {
  cout << "n = " << int(RECS_POW) << ", t = " << t1/1000 << " s, ";
  if (dbsize)
    cout << "size = " << round(dbsize/1048576.0) << " MB, ";
  cout << "Kops:\t" << opsKops(ops1*1000) << "\t" << opsKops(ops2) << "\t" << opsKops(ops3) << "\t" << opsKops(ops4) << endl;
}
#endif // COMMON_H
