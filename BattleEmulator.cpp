//
// Created by Owner on 2024/02/05.
//

#include <cstdint>
#include <array>
#include <algorithm>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "BattleEmulator.h"
#include "lcg.h"
#include "Player.h"
#include "attack/BoltCutter/BoltCutter.h"

uint64_t previousState = 0;

void BattleEmulator::Main(int *position, int Gene[], const Player players[4]) {
    int genePosition = 0;
    std::array<double, 5> speed = {34, 30, 30, 30, 45};
    for (double &element: speed) {
        element *= lcg::floatRand(position, 0.51, 1.0);
    }

    // インデックスと要素をペアにして保持
    std::vector<std::pair<double, size_t>> indexed_speed;
    for (size_t i = 0; i < speed.size(); ++i) {
        indexed_speed.emplace_back(speed[i], i);
    }

    // ラムダ式を使用して要素を比較
    auto compare_function = [](const auto &lhs, const auto &rhs) {
        return lhs.first > rhs.first;
    };

    // ソート
    std::sort(indexed_speed.begin(), indexed_speed.end(), compare_function);

    int enemyAction = FUN_0208aecc(position);
    int AITarget = -1;
    if (enemyAction == 0xf9) {
        AITarget = AttackTargetSelection(position, players);
        (*position) += 3;
    }

    int actionTable[4] = {0, 0, 0, 0};
    for (int &i: actionTable) {
        i = Gene[genePosition++];
    }


    std::cout << AITarget << std::endl;

    // ソートされた結果を出力
    for (const auto &element: indexed_speed) {
        //std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
        if (element.second == 4) {
            //--------start_FUN_02158dfc-------
            (*position) += 5;
            //--------end_FUN_02158dfc-------
            callAttackFun(AITarget, position, players);
        }
    }

}

void BattleEmulator::callAttackFun(int Id, int* position, const Player players1[4]) {
    std::unordered_map<int, std::function<void(int* pos, const Player players[4])>> jumpTable;
    // idに対応するクラスの関数を登録
    jumpTable[0xfe] = [](int* pos, const Player players[4]) { BoltCutter::onRun(pos, players); };
    //jumpTable[0x49] = [](int* pos, Player players[4]) { DerivedClass2().onRun(); };

    auto it = jumpTable.find(Id);

    if (it != jumpTable.end()) {
        // idが見つかった場合
        it->second(position, players1);
    } else {
        // idが見つからなかった場合のデフォルト処理
        std::cout << "Default action for unknown id: " << Id << std::endl;
    }
}

int BattleEmulator::AttackTargetSelection(int *position, const Player players[4]) {
    int max = 0;
    int tmp[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; ++i) {
        if (!Player::isPlayerAlive(players[i])) {
            continue;
        }
        if (players[i].guard == FRONT) {
            tmp[i] = 2;
            max += 2;
        } else {
            tmp[i] = 1;
            max += 1;
        }
    }
    int result = lcg::getPercent(position, max);
    for (int i = 0; i < 4; ++i) {
        if (!Player::isPlayerAlive(players[i])) {
            continue;
        }
        int element = tmp[i];
        if (result <= element) {
            return i;
        }
        result -= element;
    }
    throw std::logic_error("AttackTargetSelection error");
}

int BattleEmulator::FUN_0208aecc(int *position) {
    int attack[6] = {0xf9, 1, 0x49, 0x1e, 0xf9, 0x49};
    uint64_t r0_var2 = lcg::getSeed(position);
    uint64_t r3_var3 = previousState;
    uint64_t r2_var5 = r0_var2 & 0x1;
    uint64_t r0_var6 = r3_var3 << 0x1 & 0xFFFFFFFF;
    uint64_t r1_var7 = r0_var6 & 0xff;
    uint64_t r0_var8 = r3_var3 + 0x1;
    uint64_t r3_var9 = r1_var7 + r2_var5;
    previousState = r0_var8;
    uint64_t r3_var12 = r3_var9 & 0xff;
    std::cout << r3_var12 << std::endl;
    return attack[r3_var12];
}
