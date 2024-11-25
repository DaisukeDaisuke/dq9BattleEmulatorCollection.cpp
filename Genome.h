//
// Created by Owner on 2024/11/21.
//

#ifndef NEWDIRECTORY_GENOME_H
#define NEWDIRECTORY_GENOME_H

#include <cstdint>
#include "Player.h"

// 遺伝子型
struct Genome {
    Player AllyPlayer;
    Player EnemyPlayer;
    uint64_t state;
    int position;
    int actions[500];
    int EActions[2];
    int Aactions;
    int turn;
    int fitness;
};


#endif //NEWDIRECTORY_GENOME_H
