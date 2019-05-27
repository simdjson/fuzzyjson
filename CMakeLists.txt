cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (fuzzyjson)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werrror")
endif()

macro(append var string)
  set(${var} "${${var}} ${string}")
endmacro(append)
append(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
append(CMAKE_CXX_FLAGS "-g3 -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all")

include_directories("include")
include_directories("dependencies/randomjson/include")
include_directories("dependencies/simdjson/singleheader")
include_directories("dependencies/rapidjson/include")

add_subdirectory(tests)
add_test( tests tests/unittests.cpp )