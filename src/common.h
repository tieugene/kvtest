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
#if defined(__APPLE__)
#include <mach/mach.h>
#else
#include <fstream>
#endif

using namespace std;

// internal things
typedef array<uint32_t, 6> KEYTYPE_T;   ///< key type (24-bytes uint)
static uint32_t *rainbow;               ///< random uint32 numbers
const int RAINBOW_SIZE = 0x10000;       ///< 64K
// CLI options
static uint8_t RECS_POW;                ///< log2(Records to create)
static uint32_t RECS_QTY;               ///< Records to create
static uint8_t TEST_DELAY = 5;          ///< delay of each test, sec
static bool TUNING = false;
static bool verbose = false;            ///< programm verbosity
static filesystem::path dbname;         ///< database file/dir name
static bool test_get = true, test_ask = true, test_try = true;  ///< Stages to execute
// internal system-wide variables
static long mem0 = 0, mem1 = 0;             ///< resident memory used at start and during work (excl. Try())
static uint32_t t1, ops1, ops2, ops3, ops4; ///< Results: times (ms) and speeds (kilo-operations per second) for all stages
static bool can_play = false;               ///< timing trigger
static chrono::time_point<chrono::steady_clock> T0;

class KVTError : public runtime_error {
public:
  KVTError(const string& msg = "") : runtime_error(msg) {}
};

///< Error messages
const string
  Err_Cannot_New = "Cannot 'new' DB",
  Err_Cannot_Create = "Cannot create DB",
  Err_Cannot_Tune = "Cannot tune DB",
  Err_Cannot_Sync = "Cannot sync DB",
  Err_Cannot_Add = "Cannot add record",
  Err_Cannot_Get = "Cannot get record",
  Err_Unexpected_Value = "Unexpected value found",
  Err_Not_All_Add = "Not all records added",
  Err_Not_All_Get = "Not all records got"
;

///< Help
static string_view help_txt = "\
Usage: [options] <log2(records) (0..31)>\n\
Options:\n\
-h        - this help\n\
-f <path> - file/dir name of DB\n\
-t n      - duration of each test, sec (1..255, default=5)\n\
-x s      - exclude step[s] (g=Get/a=Ask/t=Try)\n\
-z        - tuning on\n\
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

  while ((opt = getopt(argc, argv, "hf:t:x:vz")) != -1) {
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
      case 'z':
        TUNING = true;
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

unsigned long get_RAM(void) {
  unsigned long ram = 0;
#if defined (__linux__)
  string token;
  ifstream meminfo("/proc/meminfo");
  while(meminfo >> token) {
      if(token == "MemTotal:") {  // kB
          unsigned long mem;
          if(meminfo >> mem)
              ram = mem;
          else
              ram = 0;
      }
      // ignore rest of the line
      meminfo.ignore(numeric_limits<std::streamsize>::max(), '\n');
  }
#elif defined(__APPLE__)
  ram = 4;  // dummy
#endif
  return ram;
}

/**
 * @brief Memory usage
 * @return Used memory in Kilobytes
 */
long get_statm(void) {
    long    total = 0, rss = 0;  // shared, text, lib, data, dt; man proc
#if defined (__linux__)
    ifstream statm("/proc/self/statm");
    statm >> total >> rss; // >> shared...
    statm.close();
    total = rss * (sysconf(_SC_PAGE_SIZE) >> 10);  // pages-ze = 4k in F32_x64
#elif defined(__APPLE__)
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (KERN_SUCCESS == task_info(mach_task_self(),
        TASK_BASIC_INFO, (task_info_t)&t_info,
        &t_info_count))
        total = t_info.resident_size >> 10;
#endif
    return total;
}

void update_mem(void) {
  mem1 = max(mem1, get_statm());
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

uint64_t f_size(const filesystem::path &path) {
  return filesystem::file_size(path);
}

uint64_t d_size(const filesystem::path &path) {
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
void stage_add(function<void (const KEYTYPE_T &, const uint32_t)> func_recadd) {
  KEYTYPE_T k;

  mem0 = get_statm();
  if (verbose)
    cerr << "Playing 2^" << int(RECS_POW) <<" = " << RECS_QTY << " records (tunig: " << (TUNING ? "ON" : "OFF") << "):" << endl
         << "1. Add ... ";
  time_start();
  for (uint32_t v = 0; v < RECS_QTY; v++) {
      get_key(v, k);
      func_recadd(k, v);
  }
  t1 = time_stop();
  ops1 = (t1) ? RECS_QTY/t1 : 0;
  if (verbose)
    cerr << RECS_QTY << " @ " << t1 << " ms (" << opsKops(ops1*1000) << " Kops)" << endl;
  update_mem();
}

/**
 * @brief Test getting TESTS_QTY existing records from DB
 * @param func_recget Callback to get a record from DB
 */
void stage_get(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  uint32_t all = 0, found = 0;
  KEYTYPE_T k;

  update_mem();
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
  if (found != all)
    throw KVTError(Err_Not_All_Get);
  ops2 = all/TEST_DELAY;
  if (verbose)
    cerr << found << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops2) << " Kops)" << endl;
  update_mem();
}

/**
 * @brief Test getting TESTS_QTY existing/random (50/50) records from DB
 * @param Callback to get a record from DB
 */
void stage_ask(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget) {
  uint32_t all = 0, found = 0, not_recs_qty = ~RECS_QTY;
  KEYTYPE_T k;

  update_mem();
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
    cerr << all << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops3) << " Kops): " << found << " = " << round(100.0*found/all) << "% found" << endl;
  update_mem();
}

/**
 * @brief Test getting existing (50%) or adding not existing (50%) TESTS_QTY records in DB
 * @param Callback to get-or-add a record in DB
 */
void stage_try(function<bool (const KEYTYPE_T &, const uint32_t)> func_rectry) {
  // FIXME: don't try to find 'unknown' keys that already added
  uint32_t all = 0, found = 0, recs_qty = RECS_QTY;
  KEYTYPE_T k;

  update_mem();
  if (verbose)
    cerr << "4. Try ... ";
  lets_play(TEST_DELAY);
  while (can_play) {
    uint32_t v = (all & 1) ? rand() % recs_qty : recs_qty;
    get_key(v, k);
    if (func_rectry(k, v))
      found++;
    else
      recs_qty++;
    all++;
  }
  ops4 = all/TEST_DELAY;
  if (verbose)
    cerr << all << " @ " << int(TEST_DELAY) << " s (" << opsKops(ops4) << " Kops): " << found << " = " << round(100.0*found/all) << "% found" << endl;
}

/**
 * @brief Output test results to stdout
 */
void out_result(uint64_t dbsize=0) {
  cout << "n = " << int(RECS_POW) << ", t = " << t1/1000 << " s, "
       << "size = " << round(dbsize/1048576.0) << " MB, "
       << "RAM = " << mem1/1024 << " MB, "
       << "Kops:\t" << opsKops(ops1*1000) << "\t" << opsKops(ops2) << "\t" << opsKops(ops3) << "\t" << opsKops(ops4) << endl;
}
#endif // COMMON_H
