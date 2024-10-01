//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

class BattleResult {

public:
    static void add(BattleResult &obj, int action, int damage, bool isEnemy, bool isParalysis, bool isInactive, int turn, bool player0_has_initiative, int ehp, int ahp) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.isEnemy[obj.position] = isEnemy;
        obj.isParalysis[obj.position] = isParalysis;
        obj.isInactive[obj.position] = isInactive;
        obj.turns[obj.position] = turn;
        obj.initiative[obj.position] = player0_has_initiative;
        obj.ehp[obj.position] = ehp;
        obj.ahp[obj.position] = ahp;
        obj.turn = turn;
        obj.position++;
    }

    int position = 0;
    int turn = 0;
    int actions[100] = {};
    int damages[100] = {};
    int isEnemy[100] = {};
    bool isParalysis[100] = {};
    bool isInactive[100] = {};
    int turns[100] = {};
    bool initiative[100] = {};
    int ehp[100] = {};
    int ahp[100] = {};
};

#endif //NEWDIRECTORY_BATTLERESULT_H
