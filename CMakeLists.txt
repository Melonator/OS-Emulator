cmake_minimum_required(VERSION 3.29)
project(OS_Emualtor)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -static")

add_executable(OS_Emualtor src/main.cpp
        include/screen.h
        include/cpu.h
        include/scheduler.h
        src/screen.cpp
        src/scheduler.cpp
        src/cpu.cpp
        include/shared_state.h
        src/shared_state.cpp
        include/color.hpp
)
