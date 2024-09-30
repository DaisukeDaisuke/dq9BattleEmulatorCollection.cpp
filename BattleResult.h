//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

#include <vector>
#include "Player.h"

class BattleResult {
public:

    static void add(BattleResult &obj, int action, int damage, bool isEnemy, const int turn, const Player& player, const Player& enemy, const bool Preemptive) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.isEnemy[obj.position] = isEnemy;

        obj.turn[obj.position] = turn;
        obj.preemptive[obj.position] = Preemptive;
        obj.totalTurn = turn;

        obj.position++;
    }

    int position = 0;
    int totalTurn = 0;
    int actions[100] = {};
    int damages[100] = {};
    int isEnemy[100] = {};
    int turn[100] = {};
    int preemptive[100] = {};

};

#endif //NEWDIRECTORY_BATTLERESULT_H