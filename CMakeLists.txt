cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH /usr/local)
endif (APPLE)

include_directories(src)

# Define k-v backends
find_package(PkgConfig REQUIRED)

# tkrzw
pkg_search_module(TK IMPORTED_TARGET tkrzw)
if (TK_FOUND)
    add_definitions(-DTKRZW)
    add_executable(kvtest_tkrzw src/tkrzw.cpp)
    target_link_libraries(kvtest_tkrzw PkgConfig::TK)
    install(TARGETS kvtest_tkrzw RUNTIME DESTINATION bin)
endif(TK_FOUND)
message(STATUS "Tkrzw: ${USE_TK}")

# kyotocabinet
pkg_search_module(KC IMPORTED_TARGET kyotocabinet)
if (KC_FOUND)
    add_definitions(-DKYOTOCABINET)
    add_executable(kvtest_kc src/kc.cpp)
    target_link_libraries(kvtest_kc PkgConfig::KC)
    install(TARGETS kvtest_kc RUNTIME DESTINATION bin)
endif(KC_FOUND)
message(STATUS "Kyotocabinet: ${USE_KC}")

# leveldb
find_package(leveldb CONFIG REQUIRED)
if (USE_LDB)
    add_definitions(-DLEVELDB)
    add_executable(kvtest_ldb src/ldb.cpp)
    if (APPLE)
        include_directories(/usr/local/opt/leveldb/include/)
        target_link_libraries(kvtest_ldb /usr/local/opt/leveldb/lib/libleveldb.dylib)
    else (APPLE)
        target_link_libraries(kvtest_ldb leveldb)
    endif (APPLE)
    install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
endif(USE_LDB)
message(STATUS "LevelDB: ${USE_LDB}")

# BerkeleyDB
# macOS: berkeley-db
# Linux: libdb-cxx-devel
find_library(BDB_LIBRARY db)
if (BDB_LIBRARY)
    add_definitions(-DBDB)
    add_executable(kvtest_bdb src/bdb.cpp)
    if (APPLE)
        include_directories(/usr/local/opt/berkeley-db/include/)
        target_link_libraries(kvtest_bdb /usr/local/opt/berkeley-db/lib/libdb_cxx.dylib)
    else (APPLE)
        target_link_libraries(kvtest_bdb db)
    endif (APPLE)
    install(TARGETS kvtest_bdb RUNTIME DESTINATION bin)
endif (BDB_LIBRARY)
message(STATUS "BerkeleyDB: ${BDB_LIBRARY}")

