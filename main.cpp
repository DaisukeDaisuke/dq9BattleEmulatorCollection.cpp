#include <iostream>
#include <ctime>        // time
#include <cstdlib>      // srand,lcg
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <chrono>
#include "lcg.h"
#include "BattleEmulator.h"

using namespace std;


int main() {
    auto t0 = std::chrono::high_resolution_clock::now();
    // Declare and initialize an array to store precalculated values
    uint64_t seed = 0x127578ED;
    lcg::init(seed);
    int *position = new int(1);
    // Now you can use the precalculated values as needed
    //int test = 0;

    Player players[5];
    int hps[5] = {51, 29, 29, 29, 296};
    int defs[5] = {31, 16, 16, 15,50};
    int atks[5] = {36, 10, 10, 10,53};
    int speeds[5] = {34, 30, 30, 30, 45};
    for (int i = 0; i < 5; ++i) {
        players[i].playerId = i;
        players[i].hp = hps[i];
        players[i].atk = atks[i];
        players[i].def = defs[i];
        players[i].speed = speeds[i];
    }

    int gene[100] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    BattleEmulator::Main(position, gene, players);



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
    lcg::release();


    return 0;
}
