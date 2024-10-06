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
    static const int PUFF_PUFF = 1011;
    static const int MANAZASHI = 1010;
    static const int VICTIMISER = 1009;
    static const int CRACK_ENEMY = 1008;
    static const int INACTIVE_ENEMY = 1013;
    static const int INACTIVE_ALLY = 1006;
    static const int HP_HOOVER = 1005;
    static const int BOLT_CUTTER = 0xf9;
    static const int ATTACK_ENEMY = 1016;
    static const int MULTITHRUST = 0x49;
    static const int MEDICINAL_HERBS = 1002;
    static const int PARALYSIS = 1017;
    static const int COUNTER = 1022;
    static const int ACROBATSTAR_KAIHI = 1023;
    static const int CURE_PARALYSIS = 1024;

    static const int ATTACK_ALLY = 1004;
    static const int HEAL = 0x1E;
    static const int ACROBATIC_STAR = 1021;
    static const int DEFENCE = 1020;

    static bool
    Main(int *position, int RunCount, std::vector<int32_t> Gene, Player *players, BattleResult &result, uint64_t seed);

    static std::string getActionName(int actionId);

private:
    static int FUN_0208aecc(int *position);

    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int AttackTargetSelection(int *position, Player *players);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static int callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender);

    static void resetCombo();

    static double processCombo(int32_t Id, double damage);

    static void ProcessFUN_021db2a0(int *position, int attacker, Player *players);

    static double FUN_021dbc04(int baseHp, double maxHp);

    static int ProcessEnemyRandomAction(int *position, int pattern);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
