//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include <optional>
#include "Player.h"
#include "BattleResult.h"

class BattleEmulator {
public:
    static const int TYPE_2A = 0;
    static const int TYPE_2B = 1;
    static const int TYPE_2C = 2;
    static const int TYPE_2E = 2;


    //2A
    static const int ATTACK_ENEMY = 1;
    static const int HIGH_SPEED_COMBO = 2;
    static const int SWITCH_2B = 3;
    static const int SWITCH_2C = 4;
    static const int LIGHTNING_STORM = 5;
    static const int CRITICAL_ATTACK = 6;

    //2C
    static const int SKY_ATTACK = 18;
    static const int MERA_ZOMA = 19;//kafrizzらしい
    static const int FREEZING_BLIZZARD = 20;//ここえるふぶき
    static const int SWITCH_2A = 20;
    static const int LULLAB_EYE = 21;//あやしいひとみ
    static const int SWITCH_2E = 22;//あやしいひとみ



    static const int INACTIVE_ENEMY = 7;
    static const int INACTIVE_ALLY = 8;

    static const int MEDICINAL_HERBS = 9;
    static const int PARALYSIS = 10;

    static const int ATTACK_ALLY = 12;
    static const int HEAL = 13;
    static const int DEFENCE = 15;

    static const int CURE_PARALYSIS = 16;
    static const int BUFF = 17;


    static bool
    Main(int *position, int startPos, int RunCount, std::vector<int32_t> Gene, Player *players,
         std::optional<BattleResult> &result, uint64_t seed, const int values[50], int maxElement, int *NowState);

    static std::string getActionName(int actionId);

private:
    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static int callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender);

    static double FUN_021dbc04(int baseHp, double maxHp);

    static int ProcessEnemyRandomAction2A(int *position);

    static int ProcessEnemyRandomAction44(int *position);
    static void process7A8(int * position,int baseDamage,Player players[2], int defender);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
