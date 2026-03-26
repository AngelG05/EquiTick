cmake_minimum_required(VERSION 3.15)
project(EquiTick LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for HFT performance
if(MSVC)
    add_compile_options(/W4 /O2 /Oi /Ot /GL)
else()
    add_compile_options(-O3 -march=native -flto -Wall -Wextra)
endif()

include_directories(include)

add_executable(equitick src/main.cpp)
