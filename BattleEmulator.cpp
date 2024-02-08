//
// Created by Owner on 2024/02/05.
//

#include <cstdint>
#include <array>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>
#include "BattleEmulator.h"
#include "lcg.h"
#include "Player.h"
#include "camera.h"
#include "debug.h"

uint64_t previousState = 0;
int32_t previousAttack = -1;
int comboCounter = 0;
bool isEnemyCombo = false;
int32_t actions[5];
int actionsPosition = 0;
int preHP[5] = {0, 0, 0, 0, 0};

void BattleEmulator::Main(int *position, const int32_t Gene[], Player *players) {
    for (int i = 0; i < 4; ++i) {
        DEBUG_COUT("> turn: " + std::to_string(i + 1));
        resetCombo();
        for (int32_t &action: actions) {
            action = -1;
        }
        actionsPosition = 0;
        int genePosition = 0;
        std::array<double, 5> speed{};
        for (int i = 0; i < 5; ++i) {
            speed[i] = players[i].speed;
        }
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
        int AITarget[4] = {-1, -1, -1, -1};
        if (enemyAction == BOLT_CUTTER) {
            AITarget[0] = AttackTargetSelection(position, players);
            (*position) += 3;
        } else if (enemyAction == MULTITHRUST) {
            int attackCount = lcg::intRangeRand(position, 3, 4);
            (*position)++;
            for (int j = 0; j < attackCount; ++j) {
                (*position)++;
            }
            (*position)++;
        } else if (enemyAction == ATTACK) {
            AITarget[0] = AttackTargetSelection(position, players);
            (*position) += 3;
        }

        int32_t actionTable[5] = {0, 0, 0, 0};
        for (auto &j: actionTable) {
            j = Gene[genePosition++];
        }

        // ソートされた結果を出力
        for (const auto &element: indexed_speed) {
            DEBUG_COUT("start: " + std::to_string(*position));
            //std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
            if (element.second == 4) {
                //--------start_FUN_02158dfc-------
                (*position) += 5;
                //--------end_FUN_02158dfc-------
                callAttackFun(enemyAction, position, players, 4, -1, AITarget);
                //--------start_FUN_021594bc-------
                (*position) += 1;
                //--------end_FUN_021594bc-------
            } else if (element.second == 0) {
                int32_t action = actionTable[element.second] & 0xffff;
                if (action == FIRE_BLOWING_ART || action == DO_YOUR_BEST) {
                    //--------start_FUN_02158dfc-------
                    (*position) += 1;
                    //--------end_FUN_02158dfc-------
                    callAttackFun(FIRE_BLOWING_ART, position, players, 0, 4, nullptr);
                    //--------start_FUN_021594bc-------
                    (*position) += 1;
                    //--------end_FUN_021594bc-------
                }
            } else {
                int32_t action = actionTable[element.second] & 0xffff;
                if (action == FIRE_BLOWING_ART) {
                    action = DO_YOUR_BEST;
                }
                if (action == DO_YOUR_BEST) {
                    //--------start_FUN_02158dfc-------
                    (*position) += 45;

                    int minIndex = 0;
                    for (int i = 1; i < 4; ++i) {
                        if (players[i].hp < players[minIndex].hp) {
                            minIndex = i;
                        }
                    }
                    if (Player::isHPBelow40Percent(players[minIndex])) {
                        action = (minIndex << 16) | MEDICINAL_HERBS;
                    } else {
                        action = MERA;
                    }
                    //--------end_FUN_02158dfc-------

                    int32_t action1 = action & 0xffff;
                    if (action1 == MERA) {
                        callAttackFun(action1, position, players, static_cast<int>(element.second), 4, nullptr);
                    } else if (action1 == MEDICINAL_HERBS) {
                        auto tmp1 = static_cast<int>((action >> 16) & 0xFFFF);
                        callAttackFun(action1, position, players, static_cast<int>(element.second), tmp1, nullptr);
                    }


                    //--------start_FUN_021594bc-------
                    (*position) += 1;
                    //--------end_FUN_021594bc-------
                }
            }
        }
        (*position) += 1;
        camera::Main(position, actions);
    }
}

void BattleEmulator::ProcessFUN_021db2a0(int *position, const int attacker, Player *players) {
    double percent = FUN_021dbc04(players[4].hp, players[4].maxHp);
    double percent1 = FUN_021dbc04(preHP[4], players[4].maxHp);
    DEBUG_COUT(std::to_string(percent1) + ", " + std::to_string(percent));
    DEBUG_COUT("hp: " + std::to_string(preHP[4]) + ", " + std::to_string(players[4].hp));
//    std::cout << percent << ", " << percent1 << std::endl;
//    std::cout << attacker << std::endl;
    if (percent1 > 0.25) {
        if (percent < 0.25) {
            //std::cout << "test1" << std::endl;
            (*position) += 2;
            return;
        }
    }
    if (percent1 > 0.5) {
        if (percent < 0.5) {
            DEBUG_COUT(" ProcessFUN_021db2a0");
            (*position) += 2;
            //FUN_021eb858
            return;
        }
    }

}

double BattleEmulator::FUN_021dbc04(int baseHp, double maxHp) {
    auto hp = static_cast<double>(baseHp);
    if (hp == 0) {
        return 0;
    }
    return hp / maxHp;
}

void BattleEmulator::callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender,
                                   const int defenders[4]) {
    for (int j = 0; j < 5; ++j) {
        preHP[j] = players[j].hp;
    }
    actions[actionsPosition++] = Id;
    int baseDamage;
    double tmp;
    bool kaisinn = false;
    bool kaihi = false;
    //int list[4] = {0,3,2,1};
    int list[4] = {0, 1, 2, 3};
    int target[4] = {-1, -1, -1, -1};
    //std::cout << Id << std::endl;
    int attackCount;
    switch (Id & 0xffff) {
        case BOLT_CUTTER://稲妻突き
            if (defenders == nullptr) {
                return;
            }
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//みわかし
            (*position)++;//盾ガード
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defenders[0]].def);
            baseDamage = floor(baseDamage * 1.2);
            (*position)++;//目を覚ました判定
            (*position)++;//不明
            Player::reduceHp(players[defenders[0]], baseDamage);
            resetCombo();
            break;
        case MULTITHRUST:
            attackCount = lcg::intRangeRand(position, 3, 4);
            (*position)++;

            for (int j = 0; j < attackCount; ++j) {
                target[j] = list[lcg::getPercent(position, 4)];
            }
            for (const auto &target1: target) {
                if (target1 == -1) {
                    break;
                }
                (*position)++;//関係ない
                (*position)++;//会心判定

                if (lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                } else {
                    (*position)++;//盾ガード
                }
                (*position)++;//回避
                baseDamage = FUN_0207564c(position, players[attacker].atk, players[target1].def);
                baseDamage *= 0.5;
                if (!kaihi) {
                    Player::reduceHp(players[target1], baseDamage);
                }
                (*position)++;//目を覚ました
                (*position)++;//必殺チャージ
            }
            resetCombo();
            break;
        case ATTACK:
            if (defenders == nullptr) {
                return;
            }
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            if (lcg::getPercent(position, 100) < 2) {
                kaihi = true;
            } else {
                (*position)++;//盾ガード
            }
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defenders[0]].def);
            (*position)++;//目を覚ました
            (*position)++;//必殺チャージ
            break;
        case FIRE_BLOWING_ART:
            if (defender == -1) {
                return;
            }
            (*position) += 2;
            (*position)++;//関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//回避

            baseDamage = FUN_021e8458_typeC(position, 20.0, 20.0, 3.0);
#ifdef DEBUG
            DEBUG_COUT(" hihuki_baseDamage: " + std::to_string(*position));
#endif
            //会心
            tmp = static_cast<double>(baseDamage);
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                //baseDamage = static_cast<int>(std::floor(tmp));
            }
            tmp *= 1.25;//レオコーン火軸性1.25倍
            baseDamage = static_cast<int>(floor(tmp));
            Player::reduceHp(players[defender], baseDamage);
#ifdef DEBUG
            DEBUG_COUT(" hihuki: " + std::to_string(baseDamage));
#endif
            (*position)++;//必殺チャージ
            //会心時不明処理
            if (kaisinn) {
                (*position) += 2;
            }
            resetCombo();
            break;
        case MERA:
            if (defender == -1) {
                return;
            }
            (*position) += 2;
            (*position)++;//関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//盾ガード
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 2, 14);
            tmp = static_cast<double>(baseDamage);
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                //baseDamage = static_cast<int>(std::floor(tmp));
            }
            tmp *= 1.25;//レオコーン火軸性1.25倍
            tmp = processCombo(Id, tmp);
            baseDamage = static_cast<int>(floor(tmp));


            (*position)++;//必殺チャージ
            //30
#ifdef DEBUG
            DEBUG_COUT(" mera: " + std::to_string(baseDamage));
#endif
            Player::reduceHp(players[defender], baseDamage);
            ProcessFUN_021db2a0(position, attacker, players);
            //会心時不明処理
            if (kaisinn) {
                (*position) += 2;
            }
            break;
        case MEDICINAL_HERBS:
            if (defender == -1) {
                return;
            }
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            baseDamage = FUN_021e8458_typeC(position, 35.0, 35.0, 5.0);
            (*position)++; // 必殺チャージ
            Player::heel(players[defender], baseDamage);
            resetCombo();
            break;
    }

}

void BattleEmulator::resetCombo() {
    previousAttack = -1;
    comboCounter = 0;
}

double BattleEmulator::processCombo(int32_t Id, double damage) {
    if (previousAttack != -1) {
        if (previousAttack == Id) {
            comboCounter++;
            switch (comboCounter) {
                case 2:
                    damage *= 1.2;
                    break;
                case 3:
                    damage *= 1.5;
                    break;
                default:
                    damage *= 2.0;
                    break;
            }
            return damage;
        } else {
            resetCombo();
        }
    }
    previousAttack = Id;
    comboCounter = 1;
    return damage;
}

int BattleEmulator::FUN_021e8458_typeC(int *position, double min, double max, double base) {
    //0x02075724
    auto result = lcg::floatRand(position, min, max);
    result += lcg::floatRand(position, -base, base);
    return static_cast<int>(floor(result));
}

int BattleEmulator::FUN_021e8458_typeD(int *position, double difference, double base) {
    //0x021e8668
    auto result = lcg::floatRand(position, -difference, difference);
    result += base;
    return static_cast<int>(floor(result));
}


int BattleEmulator::FUN_0207564c(int *position, int atk, int def) {
    auto atk5 = static_cast<double>(atk);
    auto def5 = static_cast<double>(def);

    auto atk1 = (atk5 - (def5 / 2)) / 2;
    if (atk1 <= 0) {
        return 0;
    } else {
        auto atk2 = atk / 16.0000;
        if (atk1 > atk2) {
            auto atk4 = atk1 / 16;
            auto atk3 = -atk4;
            auto result = atk1 + lcg::floatRand(position, atk3, atk4);
            result = result + lcg::floatRand(position, -1, 1);
            if (result <= 0) {
                result = 0.0;
            }
            return static_cast<int>(floor(result));
        } else {
            double result = lcg::floatRand(position, 0.0, atk2);
            if (result <= 0) {
                result = 0.0;
            }
            return static_cast<int>(floor(result));
        }
    }
    //return 0;
}

int BattleEmulator::AttackTargetSelection(int *position, Player *players) {
    int max = 0;
    int tmp[5] = {0, 0, 0, 0};
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
    int attack[6] = {BOLT_CUTTER, 1, MULTITHRUST, 0x1e, BOLT_CUTTER, MULTITHRUST};
    uint64_t r0_var2 = lcg::getSeed(position);
    uint64_t r3_var3 = previousState;
    uint64_t r2_var5 = r0_var2 & 0x1;
    uint64_t r0_var6 = r3_var3 << 0x1 & 0xFFFFFFFF;
    uint64_t r1_var7 = r0_var6 & 0xff;
    uint64_t r0_var8 = r3_var3 + 0x1;
    uint64_t r3_var9 = r1_var7 + r2_var5;
    previousState = r0_var8;
    uint64_t r3_var12 = r3_var9 & 0xff;
    return attack[r3_var12];
}
