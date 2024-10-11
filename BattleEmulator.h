//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include "Player.h"
#include "BattleResult.h"

class BattleEmulator {
public:


    static const int VICTIMISER = 0;
    static const int HP_HOOVER = 1;
    static const int CRACK_ENEMY = 2;
    static const int ATTACK_ENEMY = 3;
    static const int MANAZASHI = 4;
    static const int PUFF_PUFF = 5;
    
    static const int INACTIVE_ENEMY = 6;
    static const int INACTIVE_ALLY = 7;

    static const int MEDICINAL_HERBS = 8;
    static const int PARALYSIS = 9;
    static const int COUNTER = 10;

    static const int ATTACK_ALLY = 11;
    static const int HEAL = 12;
    static const int ACROBATIC_STAR = 13;
    static const int DEFENCE = 14;

    static const int ACROBATSTAR_KAIHI = 15;
    static const int CURE_PARALYSIS = 16;

    static bool
    Main(int *position, int RunCount, std::vector<int32_t> Gene, Player *players, BattleResult &result, uint64_t seed);

    static std::string getActionName(int actionId);

private:
    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int AttackTargetSelection(int *position, Player *players);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static int callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender);

    static double FUN_021dbc04(int baseHp, double maxHp);

    static int ProcessEnemyRandomAction(int *position, int pattern);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
