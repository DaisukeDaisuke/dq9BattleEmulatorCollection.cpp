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
bool player0_has_initiative = false;


std::string BattleEmulator::getActionName(int actionId) {

    switch (actionId) {
        case BattleEmulator::PUFF_PUFF: return "Puff Puff";
        case BattleEmulator::MANAZASHI: return "Witch's Gaze";
        case BattleEmulator::VICTIMISER: return "Victimiser";
        case BattleEmulator::CRACK_ENEMY: return "Crack";
        case BattleEmulator::INACTIVE_ENEMY: return "Inactive";
        case BattleEmulator::INACTIVE_ALLY: return "Inactive";
        case BattleEmulator::HP_HOOVER: return "HP Hoover";

        case BattleEmulator::ATTACK_ENEMY: return "Attack";
        case BattleEmulator::MULTITHRUST: return "Multithrust";
        case BattleEmulator::MEDICINAL_HERBS: return "Medicinal Herbs";
        case BattleEmulator::PARALYSIS: return "Paralysis";
        case BattleEmulator::COUNTER: return "Counter";
        case BattleEmulator::ACROBATSTAR_KAIHI: return "Acrobat Kaihi";
        case BattleEmulator::CURE_PARALYSIS: return "Cure Paralysis";

        case BattleEmulator::ATTACK_ALLY: return "Attack";
        case BattleEmulator::HEAL: return "Heal";
        case BattleEmulator::ACROBATIC_STAR: return "Acrobatic Star";
        case BattleEmulator::DEFENCE: return "Defence";

        default: return "Unknown Action";
    }
}

bool BattleEmulator::Main(int *position, int RunCount,  std::vector<int32_t> Gene, Player *players, BattleResult &result, uint64_t seed) {
    camera::reset();
    player0_has_initiative = false;
    seed1 = seed;
    previousState = 0;
    previousAttack = -1;
    comboCounter = 0;
    actionsPosition = 0;
    int damageCount = 0;
    int doAction = -1;
    int genePosition = 0;



    for (int counterJ = 0; counterJ < RunCount; ++counterJ) {
        if (players[0].dirtySpecialCharge) {
            players[0].specialCharge = false;
            players[0].dirtySpecialCharge = false;
        }
        players[0].specialChargeTurn--;
        if (players[0].specialChargeTurn == -1) {
            players[0].specialCharge = false;
        }
        DEBUG_COUT2((*position));
#ifdef DEBUG2
        if ((*position) == 230){
            std::cout << "!!" << std::endl;
        }
#endif
        int ehp = players[1].hp;
        int ahp = players[0].hp;
        bool isInactive = players[0].inactive;

//        std::cout << (*position) << ", " << players[1].hp  << ", mhp: " << players[0].hp <<  std::endl;
//        if (seed == 2501309585&&(*position) == 1012){
//            std::cout << "!!" << std::endl;
//        }
        //std::cout << players[0].specialCharge << std::endl;
//        if (counterJ == 48) {
//            //std::cout << 5 << std::endl;
//        }
//        std::cout << "hp" << std::endl;
//        std::cout << players[0].hp << std::endl;
//        std::cout << players[1].hp << std::endl;

        players[0].defence = 1.0;

        //std::cout << counterJ << std::endl;
        std::vector<std::pair<int, int>> direction;
        DEBUG_COUT("> turn: " + std::to_string(i + 1));
        resetCombo();
        for (int32_t &action: actions) {
            action = -1;
        }
        actionsPosition = 0;
        std::array<double, 2> speed{};
        for (int k = 0; k < 2; ++k) {
            speed[k] = players[k].speed;
        }
        for (double &element: speed) {
            element *= lcg::floatRand(position, 0.51, 1.0);
        }

        // インデックスと要素をペアにして保持
        std::vector<std::pair<double, size_t>> indexed_speed;
        for (size_t k = 0; k < speed.size(); ++k) {
            indexed_speed.emplace_back(speed[k], k);
        }

        // ラムダ式を使用して要素を比較
        auto compare_function = [](const auto &lhs, const auto &rhs) {
            return lhs.first > rhs.first;
        };

        // ソート
        std::sort(indexed_speed.begin(), indexed_speed.end(), compare_function);


        player0_has_initiative = (indexed_speed[0].second == 0);


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

        if (Gene.size() > genePosition&&Gene[genePosition] != 0) {
            actionTable[0] = Gene[genePosition] & 0x3ff;
            if (actionTable[0] == HEAL&&players[0].mp <= 0){
                if (players[0].medicinal_herbs_count != 0){
                    actionTable[0] = MEDICINAL_HERBS;
                    players[0].medicinal_herbs_count--;
                }else{
                    actionTable[0] = ATTACK_ALLY;
                }
            }
        }else {
            if (players[0].hp >= 35) {
                actionTable[0] = ATTACK_ALLY;
            } else {
                if (players[0].mp >= 2) {
                    actionTable[0] = HEAL;
                } else if (players[0].medicinal_herbs_count >= 1) {
                    actionTable[0] = MEDICINAL_HERBS;
                    players[0].medicinal_herbs_count--;
                } else {
                    actionTable[0] = ATTACK_ALLY;
                }
            }
            //std::cout << "hiisatu: " << (players[0].specialCharge ? "true" : "fa") << std::endl;
            if (players[0].hp >= 30) {
                if (!players[0].acrobaticStar && players[0].specialCharge && players[0].specialChargeTurn >= 0) {
                    actionTable[0] = ACROBATIC_STAR;
                }
            }
        }
        genePosition++;
        //std::cout << "hiisatu: " << (players[0].specialCharge ? "true" : "fa") << std::endl;

        //actionTable[0] = DEFENCE;

        if (actionTable[0] == DEFENCE) {
            players[0].defence = 0.5;
        }
        int enemyAction = 0;

        if (counterJ == 1) {
            //std::cout << 5 << std::endl;
        }

        // ソートされた結果を出力
        for (const auto &element: indexed_speed) {
            if (!Player::isPlayerAlive(players[1])) {
                break;
            }
            if (!Player::isPlayerAlive(players[0])) {
                break;
            }

            int basedamage = 0;
            DEBUG_COUT("start: " + std::to_string(*position));
            ////std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
            if (element.second == 1) {
                //--------start_FUN_02158dfc-------
                int table[6] = {VICTIMISER, HP_HOOVER, CRACK_ENEMY, ATTACK_ENEMY, MANAZASHI, PUFF_PUFF};
                //休み時消費:   タナトス バンパイアエッジ ヒャド 攻撃 まなざし まなざし
                //バンパイアエッジ(制限行動: タナトスハント) バンパイアエッジ ヒャド 通常攻撃 まなざし ぱふぱふ
                if (lcg::getPercent(position, 100) < 0.0160) {//0x021588ec
                    int mitore = lcg::getPercent(position, 100);//0x02158964
                    if(mitore < 90){
                        enemyAction = INACTIVE_ENEMY;
                    }
                }
                if (enemyAction != INACTIVE_ENEMY){
                    //int table[6] = {VICTIMISER, HP_HOOVER, CRACK_ENEMY, ATTACK_ENEMY, MANAZASHI, PUFF_PUFF};
                    enemyAction = table[ProcessEnemyRandomAction(position, 0)];
                    //制限行動ぱふぱふ
                    if (enemyAction == PUFF_PUFF && players[0].inactive) {
                        enemyAction = MANAZASHI;
                    }
                    if (enemyAction == MANAZASHI) {
                        //randIntRange: 0x0216139c 3 4 90
                        //randIntRange: 0x021613b0 6 8 91
                        (*position)++;
                        (*position)++;
                    } else {
                        if (!players[1].rage) {//攻撃先決定の乱数消費がなくなる
                            (*position)++;
                        }
                    }
                }
                if (enemyAction == VICTIMISER && !players[0].paralysis) {
                    enemyAction = HP_HOOVER;
                }
                if (players[1].rage) {
                    players[1].rageTurns--;
                    if (players[1].rageTurns <= 0) {
                        players[1].rage = false;
                    }
                }
                (*position)++;
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(enemyAction, position, players, 1, 0);
                BattleResult::add(result, enemyAction, basedamage, true, players[0].paralysis, isInactive || players[0].inactive, counterJ, player0_has_initiative, ehp, ahp);
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
                int32_t action = actionTable[element.second] & 0xffff;
                doAction = action;
                //--------start_FUN_02158dfc-------
                if (!players[0].paralysis) {
                    (*position) += 1;
                } else {
                    action = PARALYSIS;
                    //if (players[0].paralysisTurns != 0) {
                    players[0].paralysisTurns--;
                    //}
                    if (players[0].paralysisTurns <= 0) {
                        double paralysisTable[4] = {0.6250, 0.7500, 0.8750, 1.0000};
                        auto probability1 = paralysisTable[std::abs(players[0].paralysisTurns)];
                        auto probability2 = lcg::getPercent(position, 100) * 0.01;
                        if (probability1 >= probability2) {// 0.5 < 0.65
                            players[0].paralysis = false;
                            players[0].paralysisLevel = 0;
                            action = CURE_PARALYSIS;
                        }
                    } else {
                        (*position) += 1;
                    }
                }

                if (players[0].inactive) {
                    players[0].inactive = false;
                    if (action != CURE_PARALYSIS&&action != PARALYSIS) {
                        action = INACTIVE_ALLY;
                    }
                }


                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(action, position, players, 0, 1);
                BattleResult::add(result, action, basedamage, false,players[0].paralysis, isInactive || players[0].inactive, counterJ, player0_has_initiative, ehp, ahp);
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
                    players[0].acrobaticStarTurn--;

                    if (players[0].acrobaticStar && players[0].acrobaticStarTurn == 0) {
                        players[0].acrobaticStar = false;
                        (*position)++;//0x0215aed4 チートで100にすると次のターンから判定が行われず永遠にアクロバットスター状態になる。
                        //3f800000
                        //3f600000
                        //3f200000
                        //3ec00000
                    }
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
    if (percent1 > 0.5) {
        if (percent < 0.5) {
            DEBUG_COUT(" ProcessFUN_021db2a0");
            (*position) += 2;
            //FUN_021eb858
            return;
        }
    }
    if (percent1 > 0.5||percent1 > 0.25) {
        if (percent < 0.25) {
            ////std::cout << "test1" << std::endl;
            (*position) += 2;
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
    if (Id != ACROBATSTAR_KAIHI && Id != COUNTER) {
        actions[actionsPosition++] = Id;
    }
    int baseDamage = 0;
    double tmp = 0;
    bool kaisinn = false;
    bool kaihi = false;
    bool tate = false;
    int baseDamage_tmp = -1;
    int OffensivePower = players[attacker].atk;
    //int list[4] = {0,3,2,1};
    int list[4] = {0, 1, 2, 3};
    int target[4] = {-1, -1, -1, -1};
    ////std::cout << Id << std::endl;
    double percent = FUN_021dbc04(players[1].hp, players[1].maxHp);
    double percent1 = 0.0;
    int attackCount;
    int percent_tmp;
    //double proportionTable1[8] = {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2};
   // double proportionTable1[8] = {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 3.0, 0.2};// 21/70が2.99999...になるから最初から20/70より大きい2.89にしちゃう
    int proportionTable2[8] = {90, 90, 64, 32, 16, 8, 4, 2};
    int proportionTable3[8] = {63 , 56, 49, 42, 35, 28, 21, 14};
    switch (Id & 0xffff) {
        case BattleEmulator::MEDICINAL_HERBS:
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            baseDamage = FUN_021e8458_typeC(position, 35.0, 35.0, 5.0);
            (*position)++; // 不明
            if (!players[defender].specialCharge) {
                (*position)++; // 必殺チャージ(敵)　0%
                if (lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                    players[defender].specialCharge = true;
                    players[defender].specialChargeTurn = 6;
                }
            }
            break;
        case BattleEmulator::CURE_PARALYSIS:
            (*position) += 2;
            (*position)++;//関係ない　0x021ec6f8
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            (*position)++;//必殺チャージ(敵)　0%
            if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                players[defender].specialCharge = true;
                players[defender].specialChargeTurn = 6;
            }
            break;
        case BattleEmulator::ACROBATSTAR_KAIHI:
            //(*position) += 2;
            //(*position)++;//アクロバットスター判定
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//0x021ed7a8
            return 0;
            break;
        case BattleEmulator::COUNTER:
            //(*position) += 2;
            //(*position)++;//アクロバットスター判定
            (*position)++;//0x02158584 会心(無効)
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }
            if (lcg::getPercent(position, 100) < 2) {
                kaihi = true;
            }
            if (!kaihi) {
                (*position)++;//盾ガード
            }
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (kaisinn) {
                tmp = players[attacker].atk * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
            }
            if (!kaihi) {
                percent1 = FUN_021dbc04(preHP[1] - baseDamage, players[1].maxHp);
                if (percent1 < 0.5) {
                    if (percent >= 0.5) {
                        if (!players[1].rage) {
                            (*position)++;
                            players[1].rage = true;
                            players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                        }else{
                            (*position)++;
                        }
                    }else{
                        if (percent1 < 0.25) {
                            if (percent >= 0.25) {
                                if (!players[1].rage) {
                                    (*position)++;
                                    (*position)++;
                                } else {
                                    (*position)++;
                                }
                            }
                        }
                    }
                }

                Player::reduceHp(players[defender], baseDamage);
            }
            return 0;
            break;
        case BattleEmulator::ACROBATIC_STAR:
            players[0].acrobaticStar = true;
            players[0].acrobaticStarTurn = 6;
            if(player0_has_initiative) {
                players[0].specialCharge = false;
            }else{
                players[0].dirtySpecialCharge = true;
            }
            players[0].specialChargeTurn = 0;

            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            break;
        case BattleEmulator::DEFENCE:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            (*position)++;//必殺チャージ(敵)
            if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                players[defender].specialCharge = true;
                players[defender].specialChargeTurn = 6;
            }
            baseDamage = 0;
            break;
        case BattleEmulator::VICTIMISER:
            (*position) += 2;
            (*position)++;//アクロバットスターは絶対に発動しない
            (*position)++;//会心
            if (!players[0].paralysis && !players[0].inactive) {
                //アクロバットスターが外れた場合でもみかわしの処理は行わない
                if (!players[0].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if ( !kaihi && lcg::getPercent(position, 100) < 0.5) {
                    tate = true;
                }
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            baseDamage *= 1.5;
            if (kaihi) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else if (tate) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else {
                (*position)++;//目を覚ました
                (*position)++;//不明
                if (!players[defender].paralysis && !players[defender].inactive) {
                    //必殺チャージ(敵)
                    if (!players[defender].specialCharge) {
                        percent_tmp = lcg::getPercent(position, 100);
                        tmp = baseDamage * players[defender].defence;
                        baseDamage_tmp = static_cast<int>(floor(tmp));
                        if (baseDamage_tmp < 14) {
                            if (percent_tmp < 1) {
                                players[defender].specialCharge = true;
                                players[defender].specialChargeTurn = 6;
                            }
                        }else {
                            for (int i = 0; i < 8; ++i) {
                                if (baseDamage_tmp >= proportionTable3[i]) {
                                    if (percent_tmp < proportionTable2[i]) {
                                        players[defender].specialCharge = true;
                                        players[defender].specialChargeTurn = 6;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case BattleEmulator::ATTACK_ENEMY:
            (*position) += 2;
            if (players[0].acrobaticStar && !players[0].paralysis && !players[0].inactive) {
                percent_tmp = lcg::getPercent(position, 100);
                if (percent_tmp >= 0 && percent_tmp <= 49) {
                    return callAttackFun(ACROBATSTAR_KAIHI, position, players, attacker, defender);
                } else if (percent_tmp >= 50 && percent_tmp < 75) {
                    return callAttackFun(COUNTER, position, players, defender, attacker);
                }
            } else {
                (*position)++;
            }

            (*position)++;//会心
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[0].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi && lcg::getPercent(position, 100) < 0.5) {
                    tate = true;
                }
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            if (kaihi) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else if (tate) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else {
                (*position)++;//目を覚ました
                (*position)++;//不明
                if (!players[defender].paralysis && !players[defender].inactive) {
                    //必殺チャージ(敵)
                    if (!players[defender].specialCharge) {
                        percent_tmp = lcg::getPercent(position, 100);
                        tmp = baseDamage * players[defender].defence;
                        baseDamage_tmp = static_cast<int>(floor(tmp));
                        if (baseDamage_tmp < 14) {
                            if (percent_tmp < 1) {
                                players[defender].specialCharge = true;
                                players[defender].specialChargeTurn = 6;
                            }
                        }else {
                            for (int i = 0; i < 8; ++i) {
                                if (baseDamage_tmp >= proportionTable3[i]) {
                                    if (percent_tmp < proportionTable2[i]) {
                                        players[defender].specialCharge = true;
                                        players[defender].specialChargeTurn = 6;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case BattleEmulator::INACTIVE_ALLY:
        case BattleEmulator::PARALYSIS:
        case BattleEmulator::INACTIVE_ENEMY:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            break;
        case BattleEmulator::PUFF_PUFF:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            if (lcg::getPercent(position, 100) < 50) {//なぜか逆だけど回避50%
                players[defender].inactive = true;
            }
            if (!players[defender].inactive && players[defender].paralysis) {
                return 0;
            }
            if (players[defender].inactive) {
                FUN_0207564c(position, players[attacker].atk, players[defender].def);
                (*position)++;//不明
            } else {
                if (!players[defender].specialCharge) {
                    (*position)++;//必殺チャージ
                }
            }
            break;
        case BattleEmulator::HEAL:
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
            (*position)++;//不明
            if (!players[attacker].specialCharge) {
                (*position)++;//関係ない
            }
            //0x021eb8c8, randIntRange: 0x021eb8f0 怒り狂っている場合←の消費が発生しない。
            if (!players[1].rage) {
                (*position)++;
            }
            (*position)++;//?
            if (kaisinn) {
                if (!players[1].rage){
                    (*position)++;//会心時特殊処理　0x021e54fc
                    (*position)++;//会心時特殊処理　0x021eb8c8
                }else{
                    (*position)++;//会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }
            }
            if (!players[0].paralysis&&!players[0].inactive) {
                if (!players[attacker].specialCharge  && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].mp -= 2;
            break;
        case BattleEmulator::MANAZASHI:
            (*position) += 2;
            (*position)++;//会心
            (*position)++;//関係ない
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 4, 12);
            if (lcg::getPercent(position, 100) < 25) { // 麻痺判定
                if (players[defender].paralysisLevel == 3) {
                    //std::cerr << "paralysisLevel == 2" << std::endl;
                }
                players[defender].paralysis = true;
                players[defender].paralysisTurns = 4;
                players[defender].paralysisLevel++;
            }
            (*position)++;//不明
            if (!players[defender].paralysis && !players[defender].inactive) {
                //必殺チャージ(敵)
                if (!players[defender].specialCharge) {
                    percent_tmp = lcg::getPercent(position, 100);
                    tmp = baseDamage * players[defender].defence;
                    baseDamage_tmp = static_cast<int>(floor(tmp));
                    if (baseDamage_tmp < 14) {
                        if (percent_tmp < 1) {
                            players[defender].specialCharge = true;
                            players[defender].specialChargeTurn = 6;
                        }
                    }else {
                        for (int i = 0; i < 8; ++i) {
                            if (baseDamage_tmp >= proportionTable3[i]) {
                                if (percent_tmp < proportionTable2[i]) {
                                    players[defender].specialCharge = true;
                                    players[defender].specialChargeTurn = 6;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case BattleEmulator::CRACK_ENEMY:
            (*position) += 2;
            (*position)++;
            (*position)++;//会心
            if ((!players[0].paralysis&&!players[0].inactive) && lcg::getPercent(position, 100) < 0.5) {
                tate = true;
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 5, 17);
            if (tate) {
                baseDamage = 0;
                (*position)++;
            } else {
                (*position)++;
                if (!players[defender].paralysis && !players[0].inactive) {
                    //必殺チャージ(敵)
                    if (!players[defender].specialCharge) {
                        percent_tmp = lcg::getPercent(position, 100);
                        tmp = baseDamage * players[defender].defence;
                        baseDamage_tmp = static_cast<int>(floor(tmp));
                        if (baseDamage_tmp < 14) {
                            if (percent_tmp < 1) { // なぜか14未満の場合1%
                                players[defender].specialCharge = true;
                                players[defender].specialChargeTurn = 6;
                            }
                        }else {
                            for (int i = 0; i < 8; ++i) {
                                if (baseDamage_tmp >= proportionTable3[i]) {
                                    if (percent_tmp < proportionTable2[i]) {
                                        players[defender].specialCharge = true;
                                        players[defender].specialChargeTurn = 6;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case BattleEmulator::HP_HOOVER://バンパイアエッジ
            (*position) += 2;
            if (players[0].acrobaticStar && !players[0].paralysis && !players[0].inactive) {
                percent_tmp = lcg::getPercent(position, 100);
                if (percent_tmp >= 0 && percent_tmp <= 49) {
                    return callAttackFun(ACROBATSTAR_KAIHI, position, players, attacker, defender);
                } else if (percent_tmp >= 50 && percent_tmp < 75) {
                    return callAttackFun(COUNTER, position, players, defender, attacker);
                }
            } else {
                (*position)++;
            }
            (*position)++;//会心
            //麻痺時はみかわし、盾ガードの判定が発生しない
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[0].acrobaticStar&&lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi && lcg::getPercent(position, 100) < 0.5) {
                    tate = true;
                }
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            if (kaihi) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else if (tate) {
                (*position)++;//0x021ed7a8
                baseDamage = 0;
            } else {
                (*position)++;//目を覚ました
                (*position)++;//不明
                if (!players[defender].paralysis && !players[defender].inactive) {
                    //必殺チャージ(敵)
                    if (!players[defender].specialCharge) {
                        percent_tmp = lcg::getPercent(position, 100);
                        tmp = baseDamage * players[defender].defence;
                        baseDamage_tmp = static_cast<int>(floor(tmp));
                        if (baseDamage_tmp < 14) {
                            if (percent_tmp < 1) {
                                players[defender].specialCharge = true;
                                players[defender].specialChargeTurn = 6;
                            }
                        } else {
                            for (int i = 0; i < 8; ++i) {
                                if (baseDamage_tmp >= proportionTable3[i]) {
                                    if (percent_tmp < proportionTable2[i]) {
                                        players[defender].specialCharge = true;
                                        players[defender].specialChargeTurn = 6;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                tmp = baseDamage * players[defender].defence;
                baseDamage = static_cast<int>(floor(tmp));
                Player::heal(players[attacker], (baseDamage >> 2)); // /4
            }
            break;
        case BattleEmulator::ATTACK_ALLY:
            (*position) += 2;
            (*position)++;
            //会心
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }

            //みかわし(相手)
            if (!players[0].paralysis && !players[0].inactive) {
                if (lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi) {
                    (*position)++;//盾ガード(幼女は盾を持っていないので0%)
                }
            }
            (*position)++;//回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
//            ProcessFUN_021db2a0(position, attacker, players);
            if (kaisinn) {//0x020759ec
                tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
            }

            if (!kaihi) {
                percent1 = FUN_021dbc04(preHP[1] - baseDamage, players[1].maxHp);
                if (percent1 < 0.5) {
                    if (percent >= 0.5) {
                        if (!players[1].rage) {
                            (*position)++;
                            players[1].rage = true;
                            players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                        }else{
                            (*position)++;
                        }
                    }else{
                        if (percent1 < 0.25) {
                            if (percent >= 0.25) {
                                if (!players[1].rage) {
                                    (*position)++;
                                    (*position)++;
                                } else {
                                    (*position)++;
                                }
                            }
                        }
                    }
                }
                (*position)++;//目を覚ました
                (*position)++;//不明
            } else {
                baseDamage = 0;
            }
            if (kaisinn) {
                if (!players[1].rage){
                    (*position)++;//会心時特殊処理　0x021e54fc
                    (*position)++;//会心時特殊処理　0x021eb8c8
                }else{
                    (*position)++;//会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }

            }
            if (players[defender].hp - baseDamage >= 0) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
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

//パーセントは絶対に100%にならないから誤差-1
int BattleEmulator::FUN_021e8458_typeC(int *position, double min, double max, double base) {
    //0x02075724
    auto result = lcg::floatRand(position, min, max);
    result += lcg::floatRand(position, -base, base);
    return static_cast<int>(floor(result));
}

//パーセントは絶対に100%にならないから誤差-1
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

int BattleEmulator::FUN_0208aecc(int *position) {
    if (previousState == 3) {
        previousState = 0;
    }
    int attack[6] = {BOLT_CUTTER, ATTACK_ENEMY, MULTITHRUST, HEAL, BOLT_CUTTER, MULTITHRUST};
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


