cmake_minimum_required(VERSION 3.5)

project(kvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(src)
find_package(PkgConfig REQUIRED)

if (APPLE)
    include(macos.cmake)
else ()
    include(linux.cmake)
endif ()
