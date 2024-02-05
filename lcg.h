//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_LCG_H
#define NEWDIRECTORY_LCG_H


#include <cstdint>

class lcg {
public:
    lcg(); // コンストラクタ
    ~lcg();

    static void init(uint64_t seed);

    static uint64_t lcg_rand(uint64_t seed);

    static double calculatePercent(uint64_t input);

    static int getPercent(int *position, int max);

    static void release();

    static uint64_t getSeed(int *position);

    static double floatRand(int *position, double min, double max);
};


#endif //NEWDIRECTORY_LCG_H
