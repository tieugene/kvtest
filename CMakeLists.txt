cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(src)
find_package(PkgConfig REQUIRED)

option(USE_TK "Tkrzw" ON)
option(USE_KC "Kyotocabinet" ON)
option(USE_BDB "BerkeleyDB" ON)
option(USE_LDB "LevelDB" ON)
option(USE_RDB "RocksDB" ON)
option(USE_MDB "LMDB" OFF)

if (APPLE)
    include(macos.cmake)
else ()
    include(linux.cmake)
endif ()

message(STATUS "Tkrzw: ...... ${USE_TK}")
message(STATUS "Kyotocabinet: ${USE_KC}")
message(STATUS "BerkeleyDB: . ${USE_BDB}")
message(STATUS "LevelDB: .... ${USE_LDB}")
message(STATUS "RocksDB: .... ${USE_RDB}")
message(STATUS "LMDB: ....... ${USE_MDB}")
