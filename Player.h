//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_PLAYER_H
#define NEWDIRECTORY_PLAYER_H

enum Position {
    FRONT = 0,
    BACK = 1
};

struct Player {
    int playerId;
    int guard = FRONT;
    int hp = 0;
    int atk;
    int def;
    int speed;
    // 他のメンバー変数やメンバー関数を追加する可能性があります

    static bool isPlayerAlive(Player obj) {
        return obj.hp != 0;
    }
};

#endif //NEWDIRECTORY_PLAYER_H
