# Apple macOS

list(APPEND CMAKE_PREFIX_PATH /usr/local)

# tkrzw
# (pkg-config)
pkg_search_module(TK IMPORTED_TARGET tkrzw)
if (TK_FOUND)
    add_definitions(-DTKRZW)
    add_executable(kvtest_tkrzw src/tkrzw.cpp)
    target_link_libraries(kvtest_tkrzw PkgConfig::TK)
    install(TARGETS kvtest_tkrzw RUNTIME DESTINATION bin)
endif()
message(STATUS "Tkrzw: ${USE_TK}")

# kyotocabinet
# (pkg-config)
pkg_search_module(KC IMPORTED_TARGET kyotocabinet)
if (KC_FOUND)
    add_definitions(-DKYOTOCABINET)
    add_executable(kvtest_kc src/kc.cpp)
    target_link_libraries(kvtest_kc PkgConfig::KC)
    install(TARGETS kvtest_kc RUNTIME DESTINATION bin)
endif()
message(STATUS "Kyotocabinet: ${USE_KC}")

# leveldb
# (cmake)
find_package(leveldb CONFIG REQUIRED)
if (USE_LDB)
    add_definitions(-DLEVELDB)
    add_executable(kvtest_ldb src/ldb.cpp)
    include_directories(/usr/local/opt/leveldb/include/)
    target_link_libraries(kvtest_ldb /usr/local/opt/leveldb/lib/libleveldb.dylib)
    install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
endif()
message(STATUS "LevelDB: ${USE_LDB}")

# BerkeleyDB
# (nothing)
# macOS: berkeley-db
# Linux: libdb-cxx-devel
find_library(BDB_LIBRARY db)
if (BDB_LIBRARY)
    add_definitions(-DBDB)
    add_executable(kvtest_bdb src/bdb.cpp)
    include_directories(/usr/local/opt/berkeley-db/include/)
    target_link_libraries(kvtest_bdb /usr/local/opt/berkeley-db/lib/libdb_cxx.dylib)
    install(TARGETS kvtest_bdb RUNTIME DESTINATION bin)
endif ()
message(STATUS "BerkeleyDB: ${BDB_LIBRARY}")

# LMDB
# (macos: nothing, linux: pkg-config)
find_library(LMDB_LIBRARY lmdb)
if (LMDB_LIBRARY)
    add_definitions(-DLMDB)
    add_executable(kvtest_mdb src/mdb.cpp)
    include_directories(/usr/local/opt/lmdb/include/)
    target_link_libraries(kvtest_mdb /usr/local/opt/lmdb/lib/liblmdb.dylib)
    install(TARGETS kvtest_mdb RUNTIME DESTINATION bin)
endif ()
message(STATUS "LMDB: ${LMDB_LIBRARY}")

# RocksDB
# (cmake)
# list(APPEND CMAKE_MODULE_PATH "/usr/local/opt/rocksdb/lib/cmake")
find_package(RocksDB CONFIG)
#if (ROCKSDB_FOUND)
    add_definitions(-DROCKSDB)
    add_executable(kvtest_rdb src/rdb.cpp)
    include_directories(/usr/local/opt/rocksdb/include/)
    target_link_libraries(kvtest_rdb /usr/local/opt/rocksdb/lib/librocksdb.dylib)
    install(TARGETS kvtest_rdb RUNTIME DESTINATION bin)
#endif()
message(STATUS "RocksDB: ${ROCKSDB_FOUND}")
