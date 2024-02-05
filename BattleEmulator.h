//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include "Player.h"

class BattleEmulator {
public:
    static void Main(int *position, int *Gene, const Player *player);

    static int FUN_0208aecc(int *position);

    static int AttackTargetSelection(int *position, const Player *players);

    static int FUN_0207564c(int *position, int atk, int def);

    static void callAttackFun(int Id, int *position, const Player *players, int attacker, int defender);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
