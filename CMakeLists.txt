cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(src)
find_package(PkgConfig REQUIRED)

#option(USE_KC "Kyotocabinet" ON)
option(USE_TK "Tkrzw" ON)
option(USE_LDB "LevelDB" ON)
option(USE_BDB "BerkeleyDB" ON)
option(USE_MDB "LMDB" ON)
option(USE_RDB "RocksDB" ON)

if (APPLE)
    include(macos.cmake)
else ()
    include(linux.cmake)
endif ()

message(STATUS "BerkeleyDB:\t${USE_BDB}")
message(STATUS "LevelDB:\t\t${USE_LDB}")
message(STATUS "LMDB:\t\t${USE_MDB}")
message(STATUS "RocksDB:\t\t${USE_RDB}")
message(STATUS "Kyotocabinet:\t${USE_KC}")
message(STATUS "Tkrzw:\t\t${USE_TK}")
