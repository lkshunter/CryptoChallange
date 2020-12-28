cmake_minimum_required(VERSION 3.10)
project(CryptoChallange)

set(CMAKE_CXX_STANDARD 14)

# HIer werden die .h-Dateien von NTl angelegt
include_directories("./lib/WinNTL-11_4_3/include")

file(GLOB_RECURSE NTL_HEADER_LIST CONFIGURE_DEPENDS ${CryptoChallange_SOURCE_DIR}/lib/WinNTL-11_4_3/include/*.h)
file(GLOB_RECURSE NTL_SOURCE_LIST CONFIGURE_DEPENDS ${CryptoChallange_SOURCE_DIR}/lib/WinNTL-11_4_3/src/*.cpp)

add_library(ntl ${NTL_SOURCE_LIST} ${NTL_HEADER_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -DARMA_DONT_PRINT_CXX11_WARNING")
add_executable(CryptoChallange main.cpp)

target_link_libraries(CryptoChallange PRIVATE ntl)