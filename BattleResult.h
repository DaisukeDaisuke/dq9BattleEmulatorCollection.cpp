//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

class BattleResult {

public:
    static void add(BattleResult &obj, int action, int damage, bool isEnemy, int turn) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.isEnemy[obj.position] = isEnemy;
        obj.turn = turn;
        obj.position++;
    }

    int position = 0;
    int turn = 0;
    int actions[1000] = {};
    int damages[1000] = {};
    int isEnemy[1000] = {};
};

#endif //NEWDIRECTORY_BATTLERESULT_H
