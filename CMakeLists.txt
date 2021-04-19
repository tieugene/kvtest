cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

# Define k-v backends
find_package(PkgConfig REQUIRED)

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
