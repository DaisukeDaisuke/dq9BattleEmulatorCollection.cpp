#include <iostream>
#include <ctime>        // time
#include <cstdlib>      // srand,lcg
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <chrono>
#include <cmath>
#include "lcg.h"
#include "BattleEmulator.h"

using namespace std;


int main(int argc, char *argv[]) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int32_t gene[100] = {
            BattleEmulator::FIRE_BLOWING_ART,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::FIRE_BLOWING_ART,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::FIRE_BLOWING_ART,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::FIRE_BLOWING_ART,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST,
            BattleEmulator::DO_YOUR_BEST};

    Player players[5];
    int hps[5] = {51, 29, 29, 29, 296};
    int defs[5] = {31, 16, 16, 15,50};
    int atks[5] = {36, 10, 10, 10,53};
    int speeds[5] = {34, 30, 30, 30, 45};
    for (int i = 0; i < 5; ++i) {
        players[i].playerId = i;
        players[i].hp = hps[i];
        players[i].maxHp = hps[i];
        players[i].atk = atks[i];
        players[i].def = defs[i];
        players[i].speed = speeds[i];
    }

    Player copiedPlayers[5];
    for (int i = 0; i < 5; ++i) {
        copiedPlayers[i] = players[i];
    }

    if (argc > 1&&argv[1][0] == 'b'){
        int damages[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
        std::string speed[8] = {"-1","-1","-1","-1","-1","-1","-1","-1",};
        int counter = 0;
        for (int i = 2; i < argc; i++) {
            int convertedValue = 0;
            if (isdigit(argv[i][0])) {
                //数字
                convertedValue = std::stoi(argv[i]);
                damages[counter] = convertedValue;
                counter++;
            }else{
                std::string tmp1 = argv[i];
                speed[counter] = tmp1;
            }

        }



//        auto time = (static_cast<int>(floor(((48 * 60 + 0) * 0.19)))) << 16;
//        auto time1 = (static_cast<int>(floor(((50 * 60 + 0) * 0.19)))) << 16;
        auto time = (static_cast<int>(floor(((40 * 60 + 50) * 0.19)))) << 16;
        auto time1 = (static_cast<int>(floor(((60 * 60 + 50) * 0.19)))) << 16;

//        auto time = 0x025C5A0D-2;
//        auto time1 = 0x025C5A0D+2;

        //b z 22 A 27 17 19 28
        for (int i = time; i <time1; ++i) {
            int *position = new int(1);
            lcg::init(i, 500);
            for (int j = 0; j < 5; ++j) {
                players[j] = copiedPlayers[j];
            }
            if(BattleEmulator::Main(position, gene, players, damages, speed)){
                std::cout << "found: " << i << std::endl;
            }
            lcg::release();
            delete position;
        }

        return 0;
    }
    // Declare and initialize an array to store precalculated values
    //b a 25 16 z 22 16 22
    uint64_t seed = 36434887;
    //uint64_t seed = 0x127578ED;
    lcg::init(seed, 2000);
    int *position = new int(1);
    // Now you can use the precalculated values as needed
    //int test = 0;


    BattleEmulator::Main(position, gene, players, nullptr, nullptr);



//    for (int i = 0; i < 100; ++i) {
//        //test += lcg::getPercent(position, 100);
//        //std::cout << lcg::floatRand(position, 1.5, 2.5) << std::endl;
//    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms"
              << std::endl;
    std::cout << (*position) << std::endl;

    cout << "hp" << endl;
    for (auto & player : players) {
        std::cout << player.hp << std::endl;
    }
    lcg::release();
    std::cout << "Size of int: " << sizeof(int) * 8 << " bits" << std::endl;

    delete position;


    return 0;
}
