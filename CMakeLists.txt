cmake_minimum_required(VERSION 3.28)
project(newDirectory)

set(CMAKE_CXX_STANDARD 17)

add_executable(newDirectory main.cpp
        lcg.cpp
        lcg.h
        BattleEmulator.cpp
        BattleEmulator.h
        Player.cpp
        Player.h
        camera.cpp
        camera.h
        debug.cpp
        debug.h
        BattleResult.cpp
        BattleResult.h
        AnalyzeData.cpp
        AnalyzeData.h
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -march=x86-64 -mtune=generic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -march=x86-64 -mtune=generic")
set(CMAKE_EXE_LINKER_FLAGS "-static")