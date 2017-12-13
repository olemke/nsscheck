cmake_minimum_required(VERSION 3.7)
project(nsscheck)

set(CMAKE_CXX_STANDARD 11)

add_executable(nsscheck
        nsscheck.c
        nssdups.c
        nssdups.h
        nssgaps.c
        nssgaps.h
        nssswath.c
        nssswath.h)
