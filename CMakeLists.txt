cmake_minimum_required(VERSION 3.28)
project(newDirectory)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    # ポリシーの設定
    if(POLICY CMP0091)
        cmake_policy(SET CMP0091 NEW)  # ポリシーをNEWに設定
    endif()
endif()

#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wshadow -Wstrict-aliasing -Wunreachable-code -Wnull-dereference -Winit-self -Wfloat-equal -Wconversion -Wstrict-overflow=5 -fsanitize=undefined")
#endif()

add_executable(newDirectory main.cpp
        lcg.cpp
        lcg.h
        BattleEmulator.cpp
        BattleEmulator.h
        Player.h
        camera.cpp
        camera.h
        debug.cpp
        debug.h
        BattleResult.h
        AnalyzeData.h
        Equipment.h
        GeneticAlgorithm.cpp
        GeneticAlgorithm.h
        Genome.h
        ActionBanManager.h
)

if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -march=x86-64 -mtune=generic -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -march=x86-64 -mtune=generic -ffunction-sections -fdata-sections -fno-inline-small-functions")
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--gc-sections")
elseif(MSVC)
    set_property(TARGET newDirectory PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /O2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()
