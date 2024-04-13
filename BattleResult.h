//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

class BattleResult {

public:
    static void add(BattleResult &obj, int action, int damage) {
        obj.actions[obj.position] = action;
        obj.damages[obj.position] = damage;
        obj.position++;
    }

    int position = 0;
    int actions[1000] = {};
    int damages[1000] = {};
};

#endif //NEWDIRECTORY_BATTLERESULT_H
