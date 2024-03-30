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
#include <utility>
#include <vector>

uint64_t previousState = 0;
int32_t previousAttack = -1;
int comboCounter = 0;
bool isEnemyCombo = false;
int32_t actions[5];
int actionsPosition = 0;
int preHP[5] = {0, 0, 0, 0, 0};

bool BattleEmulator::Main(int *position, const int32_t Gene[], Player *players, const int damages[],
                          const std::string speedList[]) {
    previousState = 0;
    previousAttack = -1;
    comboCounter = 0;
    actionsPosition = 0;
    int damageCount = 0;
    int doAction = -1;
    bool love = false;
    int genePosition = 0;
    for (int i = 0; i < 4; ++i) {
        std::vector<std::pair<int, int>> direction;
        DEBUG_COUT("> turn: " + std::to_string(i + 1));
        resetCombo();
        for (int32_t &action: actions) {
            action = -1;
        }
        actionsPosition = 0;
        std::array<double, 2> speed{};
        for (int i = 0; i < 2; ++i) {
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

//        if (speedList != nullptr){
//            int checkcounter = 0;
//            for (const auto & item:indexed_speed) {
//                if(speedList[checkcounter] == "a"&&item.second != 0){
//                    return false;
//                }else if(speedList[checkcounter] == "z"&&item.second != 4){
//                    return false;
//                }
//                ++checkcounter;
//            }
//        }

        (*position)++;//0x02160d64

        int32_t actionTable[1] = {0};
        for (auto &j: actionTable) {
            j = Gene[genePosition++];
        }

        int enemyAction = 0;

        // ソートされた結果を出力
        for (const auto &element: indexed_speed) {
            int basedamage = 0;
            DEBUG_COUT("start: " + std::to_string(*position));
            //std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
            if (element.second == 1) {
                //--------start_FUN_02158dfc-------
                int table[6] = {VICTIMISER, HP_HOOVER, CRACK_ENEMY, ATTACK, UNSPEAKABLE_EVIL, PUFF_PUFF};
                //エッジ 攻撃	まなざし	ヒャド	ぱふぱふ	エッジ	タナトス
                //バンパイアエッジ(制限行動: タナトスハント) バンパイアエッジ ヒャド 通常攻撃 まなざし ぱふぱふ
                if (lcg::getPercent(position, 100) < 0.0260) {
                    love = true;
                    enemyAction = INACTIVE;
                    (*position)++;
                } else {
                    enemyAction = table[ProcessEnemyRandomAction(position, 0)];
                    if (enemyAction == UNSPEAKABLE_EVIL){
                        //randIntRange: 0x0216139c 3 4 90
                        //randIntRange: 0x021613b0 6 8 91
                        (*position)++;
                        (*position)++;
                    }else{
                        (*position)++;
                    }
                }
                if (enemyAction == VICTIMISER && !players[0].paralysis){
                    enemyAction = HP_HOOVER;
                }
                (*position)++;
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(enemyAction, position, players, 1, 0);
                std::cout << "a: " << basedamage << "\n";
                direction.emplace_back(1, 0);
                Player::reduceHp(players[0], basedamage);
                doAction = enemyAction;
                //--------start_FUN_021594bc-------
                (*position) += 1;
                //--------end_FUN_021594bc-------
            } else {
                int32_t action = actionTable[element.second] & 0xffff;
                doAction = action;
                //--------start_FUN_02158dfc-------
                (*position) += 1;
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(action, position, players, 0, 1);
                std::cout << "youzilyo: " << basedamage << "\n";
                direction.emplace_back(0, 1);
                //--------start_FUN_021594bc-------
                (*position) += 1;
                //--------end_FUN_021594bc-------
                if (action == HEAL){
                    Player::heel(players[0], basedamage);
                }else{
                    Player::reduceHp(players[1], basedamage);
                }
            }
            if (damages != nullptr) {
                if (doAction != MEDICINAL_HERBS) {
                    if (damages[damageCount] != basedamage) {
                        return false;
                    }
                    damageCount++;
                    if (damageCount == 5) {
                        //std::cout << "a" << std::endl;
                    }
                    if (damages[damageCount] == -1) {
                        return true;
                    }
                } else {
                    damageCount++;
                }
            }
        }
        (*position) += 1;
        camera::Main(position, actions, direction);
        for (int k = 0; k < 5; ++k) {
            if (players[i].hp == 0) {
                return false;
            }
        }
    }
    return false;
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

int BattleEmulator::callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender) {
    for (int j = 0; j < 2; ++j) {
        preHP[j] = players[j].hp;
    }
    actions[actionsPosition++] = Id;
    int baseDamage = 0;
    double tmp = 0;
    bool kaisinn = false;
    bool kaihi = false;
    bool tate = false;
    bool OffensivePower = players[attacker].atk;
            //int list[4] = {0,3,2,1};
    int list[4] = {0, 1, 2, 3};
    int target[4] = {-1, -1, -1, -1};
    //std::cout << Id << std::endl;
    int attackCount;
    switch (Id & 0xffff) {
        case HEAL:
            (*position)+=2;
            (*position)++;//関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 5, 35);
            if (kaisinn){
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                baseDamage = static_cast<int>(floor(tmp));
            }
            (*position)++;//不明
            (*position)++;//関係ない
            (*position)+=2;//?
            if(kaisinn) {
                (*position) += 2;//会心時特殊処理
            }
            if (lcg::getPercent(position, 100) < 1) {
                players[attacker].specialCharge = true;
            }
            break;
        case UNSPEAKABLE_EVIL:
            (*position)+=2;
            (*position)++;//会心
            (*position)++;//関係ない
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 4, 12);
            if (lcg::getPercent(position, 100) < 25){
                players[defender].paralysis = true;
            }
            (*position)++;//不明
            if (!players[defender].paralysis){
                (*position)++;// 関係ない
            }
            break;
        case CRACK_ENEMY:
            (*position)++;
            (*position)++;
            (*position)++;
            (*position)++;//会心
            if (lcg::getPercent(position, 100) < 0.5) {
                tate = true;
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 5, 17);
            if (tate){
                baseDamage = 0;
            }else{
                (*position)++;
            }
            (*position)++;
            break;
        case HP_HOOVER://バンパイアエッジ
            (*position)++;
            (*position)++;
            (*position)++;
            (*position)++;//会心
            if (lcg::getPercent(position, 100) < 2) {
                kaihi = true;
            }
            if (!kaihi&&lcg::getPercent(position, 100) < 0.5) {
                tate = true;
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (kaihi){
                (*position)++;//0x021ed7a8
            }else if (tate) {
                (*position)++;//0x021ed7a8
            }else{
                (*position)++;//目を覚ました
                (*position)++;//不明
                (*position)++;//必殺チャージ
                Player::heel(players[attacker], (baseDamage >> 2)); // /4
            }
            break;
        case ATTACK_ALLY:
            (*position)++;
            (*position)++;
            (*position)++;
            //会心
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }

            //みかわし(相手)
            if (lcg::getPercent(position, 100) < 2) {
                kaihi = true;
            }
            if (!kaihi) {
                (*position)++;//盾ガード(幼女は盾を持っていないので0%)
            }
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (kaisinn){//0x020759ec
                tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
            }
            if (!kaihi) {
                (*position)++;//目を覚ました
                (*position)++;//不明
            }
            if(kaihi){
                (*position)++;//会心時特殊処理　0x021e54fc
                (*position)++;//会心時特殊処理　0x021eb8c8
            }
            if (lcg::getPercent(position, 100) < 1) {
                players[attacker].specialCharge = true;
            }
            break;
    }
    return baseDamage;
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

int BattleEmulator::ProcessEnemyRandomAction(int *position, int pattern) {//0x0208aca8
    int patternTable[6] = {43, 42, 43, 43, 42, 43};
    int rand = lcg::getPercent(position, 0x100)+1;

    for (int i = 0; i < 6; ++i) {
        if (rand < patternTable[i]) {
            return i;
        }
        rand = rand - patternTable[i];
    }
    return 5;
}

int BattleEmulator::FUN_0208aecc(int *position) {
    if (previousState == 3) {
        previousState = 0;
    }
    int attack[6] = {BOLT_CUTTER, ATTACK, MULTITHRUST, HEAL, BOLT_CUTTER, MULTITHRUST};
    uint64_t r0_var2 = lcg::getSeed(position);
    uint64_t r3_var3 = previousState;
    uint64_t r2_var5 = r0_var2 & 0x1;
    uint64_t r0_var6 = r3_var3 << 0x1 & 0xFFFFFFFF;
    uint64_t r1_var7 = r0_var6 & 0xff;
    uint64_t r0_var8 = r3_var3 + 0x1;
    uint64_t r3_var9 = r1_var7 + r2_var5;
    previousState = r0_var8;
    uint64_t r3_var12 = r3_var9 & 0xff;
    DEBUG_COUT1(previousState);
    return attack[r3_var12];
}
