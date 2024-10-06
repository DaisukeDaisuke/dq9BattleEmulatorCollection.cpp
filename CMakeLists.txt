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

if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=x86-64 -mtune=generic -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=x86-64 -mtune=generic -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--gc-sections")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /O2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()
