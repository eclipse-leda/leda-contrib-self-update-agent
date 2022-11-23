cmake_minimum_required(VERSION 3.10)
project(sdv-self-update-agent)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ../lib)

set(SDV_SUA_BUILD_TESTS TRUE)

add_subdirectory(3rdparty)
add_subdirectory(src)

if(SDV_SUA_BUILD_TESTS)
  add_subdirectory(utest)
endif()
