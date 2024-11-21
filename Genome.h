//
// Created by Owner on 2024/11/21.
//

#ifndef NEWDIRECTORY_GENOME_H
#define NEWDIRECTORY_GENOME_H

#include <cstdint>
#include "Player.h"

// 遺伝子型
struct Genome {
    int actions[500];
    double fitness;
};


#endif //NEWDIRECTORY_GENOME_H
