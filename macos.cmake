# Apple macOS

list(APPEND CMAKE_PREFIX_PATH /usr/local)

# kyotocabinet (pkg-config)
if (NOT DEFINED USE_KC)
    message("KC not defined")
    pkg_search_module(KC IMPORTED_TARGET kyotocabinet)
    if (KC_FOUND)
        set(USE_KC ON)
    else()
        set(USE_KC OFF)
    endif()
elseif (USE_KC)
    message("KC defined")
    pkg_search_module(KC REQUIRED IMPORTED_TARGET kyotocabinet)
endif()
if (USE_KC)
    set(USE_KC ON)
    add_definitions(-DUSE_KC)
    add_executable(kvtest_kc src/kc.cpp)
    target_link_libraries(kvtest_kc PkgConfig::KC)
    install(TARGETS kvtest_kc RUNTIME DESTINATION bin)
endif()

# tkrzw (pkg-config)
if (USE_TK)
    pkg_search_module(TK IMPORTED_TARGET tkrzw)
    if (TK_FOUND)
        set(USE_TK ON)
        add_definitions(-DUSE_TK)
        add_executable(kvtest_tkrzw src/tkrzw.cpp)
        target_link_libraries(kvtest_tkrzw PkgConfig::TK)
        install(TARGETS kvtest_tkrzw RUNTIME DESTINATION bin)
    else ()
        set(USE_TK OFF)
    endif()
endif ()

# leveldb (cmake)
if (USE_LDB)
    find_package(leveldb CONFIG REQUIRED)
    if (USE_LDB)
        add_definitions(-DUSE_LDB)
        add_executable(kvtest_ldb src/ldb.cpp)
        include_directories(/usr/local/opt/leveldb/include/)
        target_link_libraries(kvtest_ldb /usr/local/opt/leveldb/lib/libleveldb.dylib)
        install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
    endif()
endif()

# BerkeleyDB (nothing)
# macOS: berkeley-db
if (USE_BDB)
    find_library(BDB_LIBRARY db)
    if (BDB_LIBRARY)
        add_definitions(-DUSE_BDB)
        add_executable(kvtest_bdb src/bdb.cpp)
        include_directories(/usr/local/opt/berkeley-db/include/)
        target_link_libraries(kvtest_bdb /usr/local/opt/berkeley-db/lib/libdb_cxx.dylib)
        install(TARGETS kvtest_bdb RUNTIME DESTINATION bin)
    else()
        set(USE_BDB OFF)
    endif ()
endif ()

# LMDB
# (macos: nothing)
if (USE_MDB)
    find_library(LMDB_LIBRARY lmdb)
    if (LMDB_LIBRARY)
        add_definitions(-DUSE_MDB)
        add_executable(kvtest_mdb src/mdb.cpp)
        include_directories(/usr/local/opt/lmdb/include/)
        target_link_libraries(kvtest_mdb /usr/local/opt/lmdb/lib/liblmdb.dylib)
        install(TARGETS kvtest_mdb RUNTIME DESTINATION bin)
    else()
        set(USE_MDB OFF)
    endif ()
endif()

# RocksDB (cmake)
# list(APPEND CMAKE_MODULE_PATH "/usr/local/opt/rocksdb/lib/cmake")
if (USE_RDB)
    find_package(RocksDB CONFIG)
    if (USE_RDB)
        add_definitions(-DUSE_RDB)
        add_executable(kvtest_rdb src/rdb.cpp)
        include_directories(/usr/local/opt/rocksdb/include/)
        target_link_libraries(kvtest_rdb /usr/local/opt/rocksdb/lib/librocksdb.dylib)
        install(TARGETS kvtest_rdb RUNTIME DESTINATION bin)
    else()
        set(USE_RDB OFF)
    endif()
endif()
