//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include "Player.h"
#include "BattleResult.h"

class BattleEmulator {
public:;
    static const int ATTACK_ENEMY = 0;
    static const int MEDICINAL_HERBS = 1;

    static const int ATTACK_ALLY = 2;
    static const int HEAL = 3;
    static const int DEFENCE = 4;
    static const int RUBBLE = 5;

    static bool Main(int *position, const int32_t *Gene, Player *players, BattleResult &result, uint64_t seed);
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
