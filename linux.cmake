# Linux

# BerkeleyDB (nothing; libdb-cxx-devel)
# Linux: libdb-cxx-devel
if (USE_BDB)
    #set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
    #find_package(BerkeleyDB QUIET)
    #if (BerkeleyDB_FOUND)
    find_library(BDB_LIBRARY db_cxx)
    if (BDB_LIBRARY)
        add_definitions(-DUSE_BDB)
        add_executable(kvtest_bdb src/bdb.cpp)
        target_link_libraries(kvtest_bdb db_cxx)
        target_link_libraries(kvtest_bdb pthread)
        target_link_libraries(kvtest_bdb common)
        install(TARGETS kvtest_bdb RUNTIME DESTINATION bin)
    else()
        set(USE_DBD OFF)
    endif ()
endif()

# leveldb (cmake; leveldb-devel)
if (USE_LDB)
    find_package(leveldb CONFIG REQUIRED)
    if (leveldb_FOUND)
        add_definitions(-DUSE_LDB)
        add_executable(kvtest_ldb src/ldb.cpp)
        target_link_libraries(kvtest_ldb leveldb)
        target_link_libraries(kvtest_ldb pthread)
        target_link_libraries(kvtest_ldb common)
        install(TARGETS kvtest_ldb RUNTIME DESTINATION bin)
    endif()
endif()

# RocksDB (pkg-config; rocksdb-devel)
if (USE_RDB)
    pkg_search_module(RDB IMPORTED_TARGET rocksdb)
    if (RDB_FOUND)
        add_definitions(-DUSE_RDB)
        add_executable(kvtest_rdb src/rdb.cpp)
        target_link_libraries(kvtest_rdb PkgConfig::RDB)
        target_link_libraries(kvtest_rdb pthread)
        target_link_libraries(kvtest_rdb common)
        install(TARGETS kvtest_rdb RUNTIME DESTINATION bin)
    else()
        set(USE_RDB OFF)
    endif()
endif()

# LMDB (pkg-config; lmdb-devel)
if (USE_MDB)
    pkg_search_module(MDB IMPORTED_TARGET lmdb)
    if (MDB_FOUND)
        add_definitions(-DUSE_MDB)
        add_executable(kvtest_mdb src/mdb.cpp)
        target_link_libraries(kvtest_mdb PkgConfig::MDB)
        target_link_libraries(kvtest_mdb pthread)
        target_link_libraries(kvtest_mdb common)
        install(TARGETS kvtest_mdb RUNTIME DESTINATION bin)
    else()
        set(USE_MBD OFF)
    endif ()
endif()

# kyotocabinet (pkg-config; kyotocabinet-devel)
if (USE_KC)
    pkg_search_module(KC IMPORTED_TARGET kyotocabinet)
    if (KC_FOUND)
        add_definitions(-DUSE_KC)
        add_executable(kvtest_kc src/kc.cpp)
        target_link_libraries(kvtest_kc PkgConfig::KC)
        target_link_libraries(kvtest_kc pthread)
        target_link_libraries(kvtest_kc common)
        install(TARGETS kvtest_kc RUNTIME DESTINATION bin)
    else ()
        set(USE_KC OFF)
    endif()
endif()

# tkrzw (pkg-config; tkrzw-devel)
if (USE_TK)
    pkg_search_module(TK IMPORTED_TARGET tkrzw)
    if (TK_FOUND)
        add_definitions(-DUSE_TK)
        add_executable(kvtest_tk src/tk.cpp)
        target_link_libraries(kvtest_tk PkgConfig::TK)
        target_link_libraries(kvtest_kc common)
        install(TARGETS kvtest_tk RUNTIME DESTINATION bin)
    else ()
        set(USE_TK OFF)
    endif()
endif()
