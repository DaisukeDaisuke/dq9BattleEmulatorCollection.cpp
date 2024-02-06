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
    static const int DO_YOUR_BEST = 1000;//パッチリがんばれ
    static const int FIRE_BLOWING_ART = 0x1001;//火吹き芸
    static const int MEDICINAL_HERBS = 0x1002;
    static const int MERA = 0x1003;
    static int FUN_0208aecc(int *position);

    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int AttackTargetSelection(int *position, Player *players);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static double processCombo(uint32_t Id, double damage);

    static void Main(int *position, const uint32_t *Gene, Player *players);

    static void callAttackFun(uint32_t Id, int *position, Player *players, int attacker, int defender, const int *defenders);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
