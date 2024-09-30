//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

#include <vector>
#include "Player.h"

class BattleResult {
public:

    std::vector<Player> players; // Heap-allocated
    std::vector<Player> enemies; // Heap-allocated

    BattleResult() : players(20000), enemies(20000) {}

    static void add(BattleResult &obj, int action, int damage, bool isEnemy, const int turn, const Player& player, const Player& enemy, const bool Preemptive) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.isEnemy[obj.position] = isEnemy;

        // プレイヤーのコピーを保存
        //obj.players[obj.position] = player;
        //obj.enemies[obj.position] = enemy;
        obj.turn[obj.position] = turn;
        obj.preemptive[obj.position] = Preemptive;
        obj.totalTurn = turn;

        obj.position++;
    }

    int position = 0;
    int totalTurn = 0;
    int actions[20000] = {};
    int damages[20000] = {};
    int isEnemy[20000] = {};
    int turn[20000] = {};
    int preemptive[20000] = {};

};

#endif //NEWDIRECTORY_BATTLERESULT_H