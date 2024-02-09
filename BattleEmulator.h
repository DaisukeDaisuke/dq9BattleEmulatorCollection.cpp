//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include "Player.h"

class BattleEmulator {
public:
    static const int BOLT_CUTTER = 0xf9;
    static const int MULTITHRUST = 0x49;
    static const int ATTACK = 0x1;
    static const int DO_YOUR_BEST = 1000;//パッチリがんばれ
    static const int FIRE_BLOWING_ART = 0x1001;//火吹き芸
    static const int MEDICINAL_HERBS = 0x1002;
    static const int MERA = 0x1003;
    static const int HEAL = 0x1E;

    static void Main(int *position, const int32_t *Gene, Player *players);

private:
    static int FUN_0208aecc(int *position);

    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int AttackTargetSelection(int *position, Player *players);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static int callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender, const int *defenders);

    static void resetCombo();

    static double processCombo(int32_t Id, double damage);

    static void ProcessFUN_021db2a0(int *position, int attacker, Player *players);

    static double FUN_021dbc04(int baseHp, double maxHp);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
