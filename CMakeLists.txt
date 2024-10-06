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

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
