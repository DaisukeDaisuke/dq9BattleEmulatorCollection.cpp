//
// Created by Owner on 2024/11/21.
//

#ifndef NEWDIRECTORY_GENETICALGORITHM_H
#define NEWDIRECTORY_GENETICALGORITHM_H


#include <cstdint>
#include "Player.h"
#include "Genome.h"

class GeneticAlgorithm {
public:
    static Genome RunGeneticAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations, const int actions[500]);
};


#endif //NEWDIRECTORY_GENETICALGORITHM_H
