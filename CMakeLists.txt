cmake_minimum_required(VERSION 3.17)
project(CryptoChallange)

set(CMAKE_CXX_STANDARD 14)

# HIer werden die .h-Dateien
include_directories("./lib/WinNTL-11_4_3/include")

file(GLOB_RECURSE NTL_HEADER_LIST CONFIGURE_DEPENDS ${CryptoChallange_SOURCE_DIR}/lib/WinNTL-11_4_3/include/*.h)
file(GLOB_RECURSE NTL_SOURCE_LIST CONFIGURE_DEPENDS ${CryptoChallange_SOURCE_DIR}/lib/WinNTL-11_4_3/src/*.cpp)

add_library(ntl ${NTL_SOURCE_LIST} ${NTL_HEADER_LIST})

SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11 -DARMA_DONT_PRINT_CXX11_WARNING")
add_executable(CryptoChallange main.cpp)

target_link_libraries(CryptoChallange PRIVATE ntl)