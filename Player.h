//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_PLAYER_H
#define NEWDIRECTORY_PLAYER_H

#include <algorithm>
#include <iostream>


struct Player {
    int hp;
    double maxHp;
    int atk;
    int defaultATK;
    int def;
    int defaultDEF;
    int speed;
    int defaultSpeed;
    int HealPower;
    int mp = 0;
    int maxMp = 0;
    bool specialCharge = false;
    bool dirtySpecialCharge = false;
    int specialChargeTurn = 0;
    bool paralysis = false;
    int paralysisLevel = 0;
    int paralysisTurns = -1;

    int SpecialMedicineCount = 3;
    double defence = 1.0;
    bool sleeping = false;
    int sleepingTurn = -1;
    int BuffLevel = 0;
    int BuffTurns = -1;
    bool hasMagicMirror = false;
    int MagicMirrorTurn = -1;
    int AtkBuffLevel = 0;
    int AtkBuffTurn = -1;
    int TensionLevel = 0;
    bool rage = false;
    int SageElixirCount = 2;
    int ElfinElixirCount = 1;
    int MagicWaterCount = 1;
    int speedTurn = -1;
    int speedLevel = 0;
    int PoisonTurn = -1;
    bool PoisonEnable = false;

    // 他のメンバー変数やメンバー関数を追加する可能性があります

    static bool isSpecialCharge(const Player obj) {
        return obj.specialCharge;
    }

    static bool isPlayerAlive(const Player obj) {
        return obj.hp != 0;
    }

    static void reduceHp(Player &obj, int amount) {
        obj.hp -= amount;
        obj.hp = std::max(0, obj.hp);
    }

    static void heal(Player &obj, int amount) {
        obj.hp += amount;
        obj.hp = std::min(static_cast<int>(obj.maxHp), obj.hp);
    }
};

#endif //NEWDIRECTORY_PLAYER_H
