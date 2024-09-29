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
    bool debugFlag = false;
    for (int counterJ = 0; counterJ < 10; ++counterJ) {
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

        int32_t actionTable[1] = {0};
        for (auto &j: actionTable) {
            j = Gene[genePosition++];
        }

        if (players[0].hp >= 10 || players[1].hp <= 8) {
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
        //actionTable[0] = DEFENCE;

        if (actionTable[0] == DEFENCE) {
            players[0].defence = 0.5;
        }


        if (debugFlag&&indexed_speed[0].second == 1&&enemyAction == RUBBLE){
            debugFlag = false;
        }
        if (indexed_speed[0].second == 1&&counterJ >= 4&&enemyAction == RUBBLE) {
            debugFlag = true;
            //std::cout << "0個目の要素のsecondが1です！" << std::endl;
            // ここで分岐処理を記述
        }

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

            //std::cout << (*position) << std::endl;
//            if ((*position) == 21){
//                std::cout << "!!" << std::endl;
//            }
            int basedamage = 0;
            DEBUG_COUT("start: " + std::to_string(*position));
            ////std::cout << "元の位置: " << element.second << ", 値: " << element.first << std::endl;
            if (element.second == 1) {
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
                int32_t action = actionTable[element.second] & 0xffff;
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
    if (Id != ACROBATSTAR_KAIHI && Id != COUNTER) {
        actions[actionsPosition++] = Id;
    }
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
    double percent = FUN_021dbc04(players[1].hp, players[1].maxHp);
    double percent1 = 0.0;
    int attackCount;
    int percent_tmp;
    bool isRage;
    switch (Id & 0xffff) {
        case MEDICINAL_HERBS:
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            baseDamage = FUN_021e8458_typeC(position, 35.0, 35.0, 5.0);
            (*position)++; // 不明
            break;
        case CURE_PARALYSIS:
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
        case ACROBATSTAR_KAIHI:
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
        case COUNTER:
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
                Player::reduceHp(players[defender], baseDamage);
            }
            return 0;
            break;
        case ACROBATIC_STAR:
            players[0].acrobaticStar = true;
            players[0].acrobaticStarTurn = 6;
            players[0].dirtySpecialCharge = true;
            players[0].specialChargeTurn = 0;

            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
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
        case VICTIMISER:
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
                if (!players[defender].paralysis && !players->inactive) {
                    if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                        players[defender].specialCharge = true;
                        players[defender].specialChargeTurn = 6;
                    }
                }
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case RUBBLE:
            (*position) += 2;

            (*position)++;//会心
            (*position)++;//?
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[0].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                (*position)++;//盾
            }
            (*position)++;//回避

            baseDamage = FUN_021e8458_typeD(position, 1, 6);

            if (kaihi) {
                (*position)++;//0x021ed7a8
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
        case INACTIVE_ALLY:
        case PARALYSIS:
        case INACTIVE_ENEMY:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            break;
        case PUFF_PUFF:
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
        case UNSPEAKABLE_EVIL:
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
            if (!players[defender].paralysis && !players->inactive) {
                if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[defender].specialCharge = true;
                    players[defender].specialChargeTurn = 6;
                }
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case CRACK_ENEMY:
            (*position) += 2;
            (*position)++;
            (*position)++;//会心
            if (!players[0].paralysis && lcg::getPercent(position, 100) < 0.5) {
                tate = true;
            }
            (*position)++;//回避
            baseDamage = FUN_021e8458_typeD(position, 5, 17);
            if (tate) {
                baseDamage = 0;
                (*position)++;
            } else {
                (*position)++;
                if (!players[defender].paralysis && !players->inactive) {
                    if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                        players[defender].specialCharge = true;
                        players[defender].specialChargeTurn = 6;
                    }
                }
            }

            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            break;
        case HP_HOOVER://バンパイアエッジ
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
                if (!players[defender].paralysis && !players->inactive) {
                    if (!players[defender].specialCharge && lcg::getPercent(position, 100) < 1) {//0x021ed7a8
                        players[defender].specialCharge = true;
                        players[defender].specialChargeTurn = 6;
                    }
                }
                tmp = baseDamage * players[defender].defence;
                baseDamage = static_cast<int>(floor(tmp));
                Player::heal(players[attacker], (baseDamage >> 2)); // /4
            }
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
            percent1 = FUN_021dbc04(preHP[1] - baseDamage, players[1].maxHp);
            if (kaisinn) {//0x020759ec
                tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
                (*position) += 2;
            }
            if (percent1 < 0.5) {
                if (percent > 0.5) {
                    (*position)++;
                    players[1].rage = true;
                    players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                }
            }
            if (percent1 < 0.25) {
                if (percent > 0.25) {
                    (*position)++;
                    players[1].rage = true;
                    players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                }
            }
//            if (isRage){
//                (*position) += 2;
//            }
//            ProcessFUN_021db2a0(position, attacker, players);
            if (!kaihi) {
                (*position)++;//目を覚ました
                (*position)++;//不明
            } else {
                baseDamage = 0;
            }

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
