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

    static void add(BattleResult &obj, int action, int damage, bool isEnemy) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.isEnemy[obj.position] = isEnemy;

        obj.position++;
    }

    int position = 0;
    int actions[20000] = {};
    int damages[20000] = {};
    int isEnemy[20000] = {};

};

#endif //NEWDIRECTORY_BATTLERESULT_H