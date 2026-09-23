set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(DEFINED ENV{CROSS_COMPILE} AND NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
    set(_cross_prefix "$ENV{CROSS_COMPILE}")
else()
    set(_cross_prefix "arm-rockchip830-linux-uclibcgnueabihf-")
endif()

if(IS_ABSOLUTE "${_cross_prefix}g++")
    set(_cxx_compiler "${_cross_prefix}g++")
else()
    find_program(_cxx_compiler NAMES "${_cross_prefix}g++")
endif()

if(NOT _cxx_compiler)
    message(FATAL_ERROR
        "C++ cross compiler not found. Set CROSS_COMPILE to the compiler prefix, "
        "for example /opt/toolchain/bin/arm-linux-uclibcgnueabihf-")
endif()

set(CMAKE_CXX_COMPILER "${_cxx_compiler}" CACHE FILEPATH "C++ cross compiler")

if(DEFINED ENV{SYSROOT} AND NOT "$ENV{SYSROOT}" STREQUAL "")
    set(CMAKE_SYSROOT "$ENV{SYSROOT}" CACHE PATH "Target sysroot")
    set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

set(_armv7_flags "-march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS_INIT "${_armv7_flags}")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
