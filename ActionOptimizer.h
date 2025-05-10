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
    static Genome RunAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[350],
                               int seedOffset,bool dropbug);
# ifdef MULTITHREADING

    static std::pair<int, Genome> RunAlgorithmAsync(const Player players[2], uint64_t seed, int turns,
                                                    int totalIterations, int actions[350], int numThreads, bool Dropbug);

private:
    static std::pair<int, Genome> RunAlgorithmSingleThread(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[], int start, int end, bool Dropbug);

#endif

private:
    static void updateCompromiseScore(Genome &genome);
};


#endif //NEWDIRECTORY_GENETICALGORITHM_H
