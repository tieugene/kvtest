cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

# Define k-v backends
find_package(PkgConfig REQUIRED)

# tkrzw
pkg_search_module(TKRZW IMPORTED_TARGET tkrzw)
if (TKRZW_FOUND)
  message(STATUS "Tkrzw: found '${TKRZW_MODULE_NAME}'")
  add_executable(kvtest_tkrzw src/tkrzw.cpp)
  target_link_libraries(kvtest_tkrzw PkgConfig::TKRZW)
  install(TARGETS kvtest_tkrzw RUNTIME DESTINATION bin)
else (TKRZW_FOUND)
  message(STATUS "Tkrzw: ---")
endif(TKRZW_FOUND)

# kyotocabinet
pkg_search_module(KC IMPORTED_TARGET kyotocabinet)
if (KC_FOUND)
  message(STATUS "Kyotocabinet: found '${KC_MODULE_NAME}'")
  add_executable(kvtest_kc src/kc.cpp)
  target_link_libraries(kvtest_kc PkgConfig::KC)
  install(TARGETS kvtest_kc RUNTIME DESTINATION bin)
else (KC_FOUND)
  message(STATUS "Kyotocabinet: ---")
endif(KC_FOUND)

# leveldb
if (APPLE)
  # brew hack
  message(STATUS "LevelDB: force")
  add_executable(kvtest_ldb src/ldb.cpp)
  include_directories(/usr/local/opt/leveldb/include/)
  target_link_libraries(kvtest_ldb /usr/local/opt/leveldb/lib/libleveldb.dylib)
  install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
else (APPLE)
pkg_search_module(LDB IMPORTED_TARGET leveldb)
if (LDB_FOUND)
  message(STATUS "LevelDB: found '${LDB_MODULE_NAME}'")
  add_executable(kvtest_ldb src/ldb.cpp)
  target_link_libraries(kvtest_ldb PkgConfig::LDB)
  install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
else (LDB_FOUND)
  message(STATUS "LevelDB: ---")
endif(LDB_FOUND)
endif(APPLE)
