cmake_minimum_required(VERSION 3.17)
project(apex_sim_2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(apex_sim_2
    apex_cpu.h
    apex_cpu.c
    apex_macros.h
    CMakeLists.txt
    file_parser.c
    new_1.asm
    main.c
    Makefile
    README.md)
