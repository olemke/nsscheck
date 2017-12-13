cmake_minimum_required(VERSION 3.7)
project(nsscheck)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)

add_executable(nsscheck
        nsscheck.c
        nssdups.c
        nssdups.h
        nssgaps.c
        nssgaps.h
        nssswath.c
        nssswath.h)
