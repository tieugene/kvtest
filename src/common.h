/**
 * kvtest - common things.
 */

#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <filesystem>
/*
#include <iostream>
#include <cstdlib>
#include <functional>
#include <cmath>     // round()
#include <chrono>
#include <thread>
*/
using namespace std;

// CLI options
extern uint32_t RECS_QTY;                 ///< Records to create
extern bool TUNING;
extern bool verbose;                      ///< programm verbosity
extern filesystem::path dbname;           ///< database file/dir name
extern bool test_get, test_ask, test_try; ///< Stages to execute

typedef array<uint32_t, 6> KEYTYPE_T;     ///< key type (24-bytes uint)
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

/**
 * @brief Process command line and reserve memory for testing keys
 * @param argc arguments number
 * @param argv arguments strings
 * @return true on siccess
 */
bool cli(int argc, char *argv[]);

/**
 * @brief Print message to stderr and return errcode
 * @param msg Message to print
 * @param err Error code to return
 * @return Error code given
 */
int ret_err(const string_view &msg, const int err);

/**
 * @brief Get RAM installed
 * @return RAM size, KB (?)
 */
unsigned long get_RAM(void);

/**
 * @brief Start timer
 */
void time_start(void);

/**
 * @brief Get duration from time_start()
 * @return Milliseconds passed from time_start()
 */
uint32_t time_stop(void);

/**
 * @brief Get file size
 * @param path File path to measure
 * @return File size
 */
uint64_t f_size(const filesystem::path &path);

/**
 * @brief Get directory size
 * @param path Directory path to measure
 * @return Directory size
 */
uint64_t d_size(const filesystem::path &path);

/**
 * @brief Create DB and add RECS_QTY testing records in it
 * @param func_recadd Callback to add a record into DB
 */
void stage_add(function<void (const KEYTYPE_T &, const uint32_t)> func_recadd);

/**
 * @brief Test getting TESTS_QTY existing records from DB
 * @param func_recget Callback to get a record from DB
 */
void stage_get(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget);

/**
 * @brief Test getting TESTS_QTY existing/random (50/50) records from DB
 * @param Callback to get a record from DB
 */
void stage_ask(function<bool (const KEYTYPE_T &, const uint32_t)> func_recget);

/**
 * @brief Test getting existing (50%) or adding not existing (50%) TESTS_QTY records in DB
 * @param Callback to get-or-add a record in DB
 */
void stage_try(function<bool (const KEYTYPE_T &, const uint32_t)> func_rectry);

/**
 * @brief Output test results to stdout
 */
void out_result(uint64_t dbsize=0);

#endif // COMMON_H
