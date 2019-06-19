cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (fuzzyjson)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werrror")
endif()

macro(append var string)
  set(${var} "${${var}} ${string}")
endmacro(append)

append(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
append(CMAKE_CXX_FLAGS "-g3 -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all")
endif()

# some compilers like clang do not automagically define __AVX2__ and __BMI2__ even when the hardware supports it
if(NOT MSVC)
   set (OPT_FLAGS "${OPT_FLAGS} -mavx2 -mbmi -mbmi2 -mpclmul")
else()
   set (OPT_FLAGS "${OPT_FLAGS} /arch:AVX2 /std:c++latest")
endif()

append(CMAKE_CXX_FLAGS ${OPT_FLAGS})


add_executable(fuzzytest src/fuzzytest.cpp)

include_directories("include")
include_directories("dependencies/args")
include_directories("dependencies/randomjson/include")
include_directories("dependencies/simdjson/singleheader")
include_directories("dependencies/rapidjson/include")


add_subdirectory(tests)
add_test(tests tests/unittests.cpp )