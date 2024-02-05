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

    Player players[4];
    int hps[4] = {51, 29, 29, 29};
    for (int i = 0; i < 4; ++i) {
        players[i].playerId = i;
        players[i].hp = hps[i];

    }

    int gene[100] = {0, 0,0,0,0,0,0,0,0};
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
    //std::cout << test << std::endl;
    lcg::release();


    return 0;
}
