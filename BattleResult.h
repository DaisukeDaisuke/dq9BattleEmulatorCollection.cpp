//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

class BattleResult {

public:
    static void
    add(std::optional<BattleResult> &obj1, int action, int damage, bool isEnemy, bool isParalysis, bool isInactive, int turn,
        bool player0_has_initiative, int ehp, int ahp) {
        if (obj1.has_value()) {
            BattleResult& obj = obj1.value();
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
    }

    int position = 0;
    int turn = 0;
    int actions[1000] = {};
    int damages[1000] = {};
    int isEnemy[1000] = {};
    bool isParalysis[1000] = {};
    bool isInactive[1000] = {};
    int turns[1000] = {};
    bool initiative[1000] = {};
    int ehp[1000] = {};
    int ahp[1000] = {};
};

#endif //NEWDIRECTORY_BATTLERESULT_H
