//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

#include <vector>
#include "Player.h"

class BattleResult {
public:

    static void add(BattleResult &obj, int action, int damage, bool isEnemy) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        //obj.isEnemy[obj.position] = isEnemy;

        obj.position++;
    }

    int position = 0;
    int actions[200] = {};
    int damages[200] = {};
    //int isEnemy[200] = {};

};

#endif //NEWDIRECTORY_BATTLERESULT_H