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
#include "BattleResult.h"
#include <utility>

uint64_t previousState = 0;
int32_t previousAttack = -1;
int comboCounter = 0;
bool isEnemyCombo = false;
int32_t actions[5];
int actionsPosition = 0;
int preHP[5] = {0, 0, 0, 0, 0};
uint64_t seed1 = 0;

bool BattleEmulator::Main(int *position, const int32_t Gene[], Player *players, BattleResult &result, uint64_t seed) {
    camera::reset();
    seed1 = seed;
    previousState = 0;
    previousAttack = -1;
    comboCounter = 0;
    actionsPosition = 0;
    int damageCount = 0;
    int doAction = -1;
    int genePosition = 0;
    for (int counterJ = 0; counterJ < 1000; ++counterJ) {
        players[0].defence = 1.0;

        //std::cout << counterJ << std::endl;
        std::vector<std::pair<int, int>> direction;
        DEBUG_COUT("> turn: " + std::to_string(i + 1));
        resetCombo();
        for (int32_t &action: actions) {
            action = -1;
        }
//        std::cout << "!!" <<  (*position) << std::endl;
//        if ((*position) == 293){
//            std::cout << "!!" << std::endl;
//        }

        actionsPosition = 0;
        double speed0 = players[0].speed * lcg::floatRand(position, 0.51, 1.0);
        double speed1 = players[1].speed * lcg::floatRand(position, 0.51, 1.0);

        bool player0_has_initiative = false;
        // 素早さを比較
        if (speed0 > speed1) {
            player0_has_initiative = true;
        }


        int table[6] = {ATTACK_ENEMY, RUBBLE, ATTACK_ENEMY, RUBBLE, ATTACK_ENEMY, ATTACK_ENEMY};
        int enemyAction = table[ProcessEnemyRandomAction(position, 0)];


        if (enemyAction == ATTACK_ENEMY){
            (*position) += 3;
            //(*position)++;
            /*
            0x02156874 0x00000002 4
            randIntRange: 0x0216139c 3 4 5
            randIntRange: 0x021613b0 6 8 6
            */
        }else if(enemyAction == RUBBLE){
            (*position) += 2;
        }

        (*position)++;//0x02160d64

        int32_t actionTable = -1;

        if (players[0].hp >= 10 || players[1].hp <= 9) {
            actionTable = ATTACK_ALLY;
        } else {
            if (players[0].mp >= 2) {
                actionTable = HEAL;
            } else if (players[0].medicinal_herbs_count >= 1) {
                actionTable = MEDICINAL_HERBS;
                players[0].medicinal_herbs_count--;
            } else {
                actionTable = ATTACK_ALLY;
            }
        }
        //actionTable = DEFENCE;

        if (actionTable == DEFENCE) {
            players[0].defence = 0.5;
        }


        if (counterJ == 1) {
            //std::cout << 5 << std::endl;
        }

        // ソートされた結果を出力
        for (int t = 0; t < 2; ++t) {
            if (!Player::isPlayerAlive(players[1])) {
                break;
            }
            if (!Player::isPlayerAlive(players[0])) {
                break;
            }

            //std::cout << (*position) << std::endl;
//            if ((*position) == 21){
//                std::cout << "!!" << std::endl;
//            }
            int basedamage = 0;
            DEBUG_COUT("start: " + std::to_string(*position));
            ////std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
            if ((t == 0 && !player0_has_initiative) || (t == 1 && player0_has_initiative)) {
                //--------start_FUN_02158dfc-------
                // 敵の準備
                (*position)+= 2;
                if (players[1].rage) {
                    players[1].rageTurns--;
                    if (players[1].rageTurns <= 0) {
                        players[1].rage = false;
                    }
                }
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(enemyAction, position, players, 1, 0);
                BattleResult::add(result, enemyAction, basedamage, true);
                //std::cout << "a: " << basedamage << "\n";
                direction.emplace_back(1, 0);
                Player::reduceHp(players[0], basedamage);
                doAction = enemyAction;
                //--------start_FUN_021594bc-------
                if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                    (*position) += 1;
                }
                //std::cout << "attack pos: " << (*position) << "\n";
                //--------end_FUN_021594bc-------
            } else {
                int32_t action = actionTable & 0xffff;
                doAction = action;
                //--------start_FUN_02158dfc-------
                (*position) += 1;
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(action, position, players, 0, 1);
                BattleResult::add(result, action, basedamage, false);
                //std::cout << "youzilyo: " << basedamage << std::endl;
                direction.emplace_back(0, 1);
                if (action == HEAL || action == MEDICINAL_HERBS) {
                    Player::heal(players[0], basedamage);
                } else {
                    Player::reduceHp(players[1], basedamage);
                }
                //--------start_FUN_021594bc-------
                //std::cout << "youzilyo pos: " << (*position) << "\n";
                if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                    (*position) += 1;
                }
            }
            //--------end_FUN_021594bc-------
        }
        //std::cout << "end turns: " << (*position) << std::endl;
        if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
            (*position) += 1;
        }
        if (counterJ == 46) {
            //std::cout << 5 << std::endl;
        }
        //std::cout << "Before camera: " << (*position) << std::endl;
        camera::Main(position, actions, direction);
        //std::cout << counterJ << std::endl;
        //std::cout << "after camera: " << (*position) << std::endl;
        //std::cout << (players[0].specialCharge ? "hissatu: true" : "hissatu: fa") << std::endl;

        if (!Player::isPlayerAlive(players[1])) {
            return false;
        }
        if (!Player::isPlayerAlive(players[0])) {
            return false;
        }
    }
    return false;
}

void BattleEmulator::ProcessFUN_021db2a0(int *position, const int attacker, Player *players) {
    double percent = FUN_021dbc04(players[1].hp, players[1].maxHp);
    double percent1 = FUN_021dbc04(preHP[1], players[1].maxHp);
    DEBUG_COUT(std::to_string(percent1) + ", " + std::to_string(percent));
    DEBUG_COUT("hp: " + std::to_string(preHP[4]) + ", " + std::to_string(players[4].hp));
//    //std::cout << percent << ", " << percent1 << std::endl;
//    //std::cout << attacker << std::endl;
    if (percent1 > 0.25) {
        if (percent < 0.25) {
            ////std::cout << "test1" << std::endl;
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
    int OffensivePower = players[attacker].atk;
    //int list[4] = {0,3,2,1};
    int list[4] = {0, 1, 2, 3};
    int target[4] = {-1, -1, -1, -1};
    ////std::cout << Id << std::endl;
    double percent1 = 0.0;
    int attackCount;
    int percent_tmp;
    switch (Id & 0xffff) {
        case MEDICINAL_HERBS:
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            baseDamage = FUN_021e8458_typeC(position, 35.0, 35.0, 5.0);
            (*position)++; // 不明
            break;
        case DEFENCE:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            (*position)++;//必殺チャージ(敵)
            if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                players[defender].specialCharge = true;
                players[defender].specialChargeTurn = 6;
            }
            break;
        case RUBBLE:
            (*position) += 2;

            (*position)++;//会心
            (*position)++;//?
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[0].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }else{
                    (*position)++;//盾
                }
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 1, 6);

            if (kaihi) {
                //(*position)++;//0x021ed7a8
                baseDamage = 0;
            } else if (tate) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else {
                (*position)++;//目を覚ました
                (*position)++;//不明
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case ATTACK_ENEMY:
            (*position) += 2;

            if (!players[1].rage){
                (*position)++;
            }

            (*position)++;//会心

            if (players[1].rage){
                (*position)++;
            }

            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[0].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }else{
                    (*position)++;//盾
                }
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            if (kaihi) {
                //(*position)++;//0x021ed7a8
                baseDamage = 0;
            } else if (tate) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else {
                (*position)++;//目を覚ました
                (*position)++;//不明
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case HEAL:
            (*position) += 2;
            (*position)++;//関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 5, 35);
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                baseDamage = static_cast<int>(floor(tmp));
            }
            (*position)+= 2;//不明
            //0x021eb8c8, randIntRange: 0x021eb8f0 怒り狂っている場合←の消費が発生しない。
            //if (!players[1].rage) {
            (*position)++;
            //}
            if (kaisinn) {
                (*position) += 2;//会心時特殊処理
            }
            players[attacker].mp -= 2;
            break;
        case ATTACK_ALLY:
            (*position) += 2;
            (*position)++;
            //会心
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }

            //みかわし(相手)
            if (!players[0].paralysis && !players[0].inactive) {
                (*position)++;
                (*position)++;//盾ガード(幼女は盾を持っていないので0%)
            }
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (kaisinn) {//0x020759ec
                tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
            }
            percent1 = FUN_021dbc04(preHP[1] - baseDamage, players[1].maxHp);
            if (percent1 < 0.5) {
                double percent = FUN_021dbc04(preHP[1], players[1].maxHp);
                if (percent > 0.5) {
                    (*position)++;
                    players[1].rage = true;
                    players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                }
            }
            if (percent1 < 0.25) {
                double percent = FUN_021dbc04(preHP[1], players[1].maxHp);
                if (percent > 0.25) {
                    (*position)++;
                    players[1].rage = true;
                    players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                }
            }
            (*position)++;//目を覚ました
            (*position)++;//不明

            if (kaisinn) {
                (*position)++;//会心時特殊処理　0x021e54fc
                (*position)++;//会心時特殊処理　0x021eb8c8
            }
            //std::cout << (players[attacker].specialCharge ? "true" : "fa") << std::endl;
            break;
        default:
            std::cout << "error!!!!! " << (Id & 0xffff) << std::endl;
    }
    return baseDamage;
}

void BattleEmulator::resetCombo() {
    previousAttack = -1;
    comboCounter = 0;
}

int BattleEmulator::ProcessEnemyRandomAction(int *position, int pattern) {//0x0208aca8
    int patternTable[6] = {43, 42, 43, 43, 42, 43};
    //125
    //43+42+43+43
    int rand = lcg::getPercent(position, 0x100) + 1;

    //std::cout << "actions rand: " << rand << std::endl;

    for (int i = 0; i < 6; ++i) {
        if (rand <= patternTable[i]) {
            return i;
        }
        rand = rand - patternTable[i];
    }
    return 5;
}
