//
// Created by Owner on 2024/11/21.
//

#ifndef NEWDIRECTORY_GENETICALGORITHM_H
#define NEWDIRECTORY_GENETICALGORITHM_H


#include <cstdint>
#include "Player.h"
#include "Genome.h"

class ActionOptimizer {
public:
    static Genome RunAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[350], int seedOffset);
    static Genome RunAlgorithmAsync(const Player players[2], uint64_t seed, int turns, int totalIterations, int actions[350]);
private:
    static Genome RunAlgorithmSingleThread(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[], int start, int end);
};


#endif //NEWDIRECTORY_GENETICALGORITHM_H
