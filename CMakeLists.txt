cmake_minimum_required(VERSION 3.16)

project(snowboy_kws VERSION 0.1.0 LANGUAGES CXX)

option(SNOWBOY_KWS_BUILD_EXAMPLES "Build the file detection example" ON)

set(SNOWBOY_KWS_SNOWBOY_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowboy/include"
    CACHE PATH "Directory containing snowboy-detect.h")
set(SNOWBOY_KWS_SNOWBOY_LIBRARY
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowboy/lib/armv7/libsnowboy-detect.a"
    CACHE FILEPATH "Snowboy static library")
set(SNOWBOY_KWS_OPENBLAS_LIBRARY
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openblas/lib/armv7/libopenblas_armv7-r0.3.21.a"
    CACHE FILEPATH "OpenBLAS static library used by Snowboy")

foreach(required_path IN ITEMS
        "${SNOWBOY_KWS_SNOWBOY_INCLUDE_DIR}/snowboy-detect.h"
        "${SNOWBOY_KWS_SNOWBOY_LIBRARY}"
        "${SNOWBOY_KWS_OPENBLAS_LIBRARY}")
    if(NOT EXISTS "${required_path}")
        message(FATAL_ERROR "Required Snowboy KWS dependency not found: ${required_path}")
    endif()
endforeach()

find_package(Threads REQUIRED)

add_library(snowboy_engine STATIC IMPORTED GLOBAL)
set_target_properties(snowboy_engine PROPERTIES
    IMPORTED_LOCATION "${SNOWBOY_KWS_SNOWBOY_LIBRARY}")

add_library(snowboy_openblas STATIC IMPORTED GLOBAL)
set_target_properties(snowboy_openblas PROPERTIES
    IMPORTED_LOCATION "${SNOWBOY_KWS_OPENBLAS_LIBRARY}")

add_library(snowboy_kws STATIC src/detector.cpp)
add_library(snowboy_kws::snowboy_kws ALIAS snowboy_kws)

target_compile_features(snowboy_kws PUBLIC cxx_std_11)
target_compile_definitions(snowboy_kws PRIVATE _GLIBCXX_USE_CXX11_ABI=0)
target_compile_options(snowboy_kws PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(snowboy_kws
    PRIVATE "${SNOWBOY_KWS_SNOWBOY_INCLUDE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(snowboy_kws PUBLIC
    snowboy_engine
    snowboy_openblas
    Threads::Threads
    ${CMAKE_DL_LIBS}
    m
)

if(SNOWBOY_KWS_BUILD_EXAMPLES)
    add_executable(snowboy_kws_file examples/file_detector.cpp)
    target_compile_features(snowboy_kws_file PRIVATE cxx_std_11)
    target_compile_options(snowboy_kws_file PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(snowboy_kws_file PRIVATE snowboy_kws::snowboy_kws)
endif()
