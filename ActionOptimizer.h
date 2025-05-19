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
    //dropBugは探索放棄バグを発生させてもいいかどうか。発生する場合より効率的な最適解が出るが、稀に探索放棄しちゃうことがある。偽だと最適解が長くなるが、探索放棄バグは発生しなくなる。
    static Genome RunAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[350], int seedOffset, bool dropBug);
};


#endif //NEWDIRECTORY_GENETICALGORITHM_H
