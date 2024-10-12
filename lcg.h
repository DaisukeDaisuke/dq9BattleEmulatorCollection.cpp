//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_LCG_H
#define NEWDIRECTORY_LCG_H


#include <cstdint>

class lcg {
private:
    static void GenerateifNeed(int need);

    static uint64_t lcg_rand(uint64_t seed);

    static double calculatePercent(uint64_t input);
public:
    static void init(uint64_t seed);

    static int getPercent(int *position, int max);

    static uint64_t getSeed(int *position);

    static double floatRand(int *position, double min, double max);

    static int intRangeRand(int *position, int min, int max);
};


#endif //NEWDIRECTORY_LCG_H
