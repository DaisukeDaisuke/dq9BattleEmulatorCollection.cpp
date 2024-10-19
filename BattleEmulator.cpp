//
// Created by Owner on 2024/02/05.
//

#include <cstdint>
#include <vector>
#include <iostream>
#include <cmath>
#include "BattleEmulator.h"
#include "lcg.h"
#include "Player.h"
#include "camera.h"
#include "debug.h"
#include "BattleResult.h"


int32_t actions[3];
int actionsPosition = 0;
int preHP[3] = {0, 0, 0};
bool player0_has_initiative = false;


std::string BattleEmulator::getActionName(int actionId) {
    switch (actionId) {
        case BattleEmulator::BUFF:
            return "Buff";
        case BattleEmulator::ATTACK_ENEMY:
            return "Attack";
        case BattleEmulator::MEDICINAL_HERBS:
            return "Medicinal Herbs";
        case BattleEmulator::PARALYSIS:
            return "Paralysis";
        case BattleEmulator::CURE_PARALYSIS:
            return "Cure Paralysis";
        case BattleEmulator::HIGH_SPEED_COMBO:
            return "uh speed combo";
        case BattleEmulator::SKY_ATTACK:
            return "sky attack";

        case BattleEmulator::ATTACK_ALLY:
            return "Attack";
        case BattleEmulator::HEAL:
            return "Heal";
        case BattleEmulator::DEFENCE:
            return "Defence";

        default:
            return "Unknown Action";
    }
}

bool BattleEmulator::Main(int *position, int startPos, int RunCount, std::vector<int32_t> Gene, Player *players, std::optional<BattleResult> &result,
                          uint64_t seed, const int values[50], int maxElement, int *NowState) {
    camera::reset();
    player0_has_initiative = false;
    actionsPosition = 0;
    int doAction = -1; // デバッグ用
    int genePosition = 0;
    int exCounter = 0;
    for (int counterJ = startPos; counterJ < RunCount; ++counterJ) {
        if (players[0].dirtySpecialCharge) {
            players[0].specialCharge = false;
            players[0].dirtySpecialCharge = false;
        }
        players[0].specialChargeTurn--;
        if (players[0].specialChargeTurn == -1) {
            players[0].specialCharge = false;
        }
#ifdef DEBUG2
        DEBUG_COUT2((*position));
        if ((*position) == 51){
            std::cout << "!!" << std::endl;
        }
#endif
        int ehp = players[1].hp;
        int ahp = players[0].hp;
        bool isInactive = players[0].inactive;

        players[0].defence = 1.0;

        for (int32_t &action: actions) {
            action = -1;
        }
        actionsPosition = 0;
        double speed0 = players[0].speed * lcg::floatRand(position, 0.51, 1.0);
        double speed1 = players[1].speed * lcg::floatRand(position, 0.51, 1.0);

        // 素早さを比較
        if (speed0 > speed1) {
            player0_has_initiative = true;
        } else {
            player0_has_initiative = false;
        }

        auto counter = 0;
        int enemyAction[2] = {0,0};
        while (counter != 2) {
            auto state = (*NowState) & 0xff;
            if (state == TYPE_2A) {
                const int table[6] = {ATTACK_ENEMY, HIGH_SPEED_COMBO, SWITCH_2B, SWITCH_2C, LIGHTNING_STORM, CRITICAL_ATTACK};
                enemyAction[counter] = table[ProcessEnemyRandomAction44(position)];
                if (enemyAction[counter] == SWITCH_2B){
                    (*NowState) = TYPE_2B; // 他のステートはリセットされるべきであって、前のサブステートを引き継ぐ理由はない。
                    (*position) += 2;
                    //TODO
                    continue;
                }else if(enemyAction[counter] == SWITCH_2C){
                    (*NowState) = TYPE_2C;
                    (*position) += 2;
                    //TODO
                    continue;
                }
                if(enemyAction[counter] == ATTACK_ENEMY){
                    (*position) += 3;
                }
                if (enemyAction[counter] == HIGH_SPEED_COMBO){
                    (*position) += 4;
                }
            }else if(state == TYPE_2C){
                const int table[6] = {SKY_ATTACK, MERA_ZOMA, FREEZING_BLIZZARD, SWITCH_2A, LULLAB_EYE, SWITCH_2E};
                enemyAction[counter] = table[ProcessEnemyRandomAction44(position)];
                if (enemyAction[counter] == SWITCH_2A){
                    (*NowState) = TYPE_2A; // 他のステートはリセットされるべきであって、前のサブステートを引き継ぐ理由はない。
                    (*position) += 2;
                    //TODO
                    continue;
                }else if(enemyAction[counter] == SWITCH_2E){
                    (*NowState) = TYPE_2E;
                    (*position) += 2;
                    //TODO
                    continue;
                }
                if (enemyAction[counter] == SKY_ATTACK){
                    (*position)++;
                }
            }
            if (counter == 0){
                (*position)++;
            }
            counter++;
        }

        int32_t actionTable = -1;

        if (Gene.size() > genePosition && Gene[genePosition] != 0) {
            actionTable = Gene[genePosition] & 0x3ff;
            if (actionTable == HEAL && players[0].mp <= 0) {
                if (players[0].medicinal_herbs_count >= 1) {
                    actionTable = MEDICINAL_HERBS;
                } else {
                    actionTable = ATTACK_ALLY;
                }
            }
        } else {
            if (players[0].hp >= 35) {
                actionTable = ATTACK_ALLY;
            } else {
                if (players[0].mp >= 2) {
                    actionTable = HEAL;
                } else if (players[0].medicinal_herbs_count >= 1) {
                    actionTable = MEDICINAL_HERBS;
                } else {
                    actionTable = ATTACK_ALLY;
                }
            }
        }
        genePosition++;

        if (actionTable == DEFENCE) {
            players[0].defence = 0.5;
        }

        // ソートされた結果を出力
        for (int t = 0; t < 2; ++t) {
            if (!Player::isPlayerAlive(players[1])) {
                break;
            }
            if (!Player::isPlayerAlive(players[0])) {
                break;
            }

            int basedamage = 0;
            if ((t == 0 && !player0_has_initiative) || (t == 1 && player0_has_initiative)) {
                for (int c = 0; c < 2; ++c) {
                    //--------start_FUN_02158dfc-------
                    (*position)++;
                    //--------end_FUN_02158dfc-------
                    basedamage = callAttackFun(enemyAction[c], position, players, 1, 0);

                    if (maxElement != -1) {
                        if (basedamage != 0 && values[exCounter++] != basedamage) {
                            return false;
                        }
                        if (maxElement <= exCounter) {
                            return true;
                        }
                    } else {
                        BattleResult::add(result, enemyAction[c], basedamage, true, players[0].paralysis,
                                          isInactive || players[0].inactive, counterJ, player0_has_initiative, ehp,
                                          ahp);
                    }
                    Player::reduceHp(players[0], basedamage);
                    //--------start_FUN_021594bc-------
                    if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                        (*position) += 1;
                    }
                    //--------end_FUN_021594bc-------
                }
            } else {
                int32_t action = actionTable & 0xffff;
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


                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(action, position, players, 0, 1);
                if (maxElement == -1) {
                    BattleResult::add(result, action, basedamage, false, players[0].paralysis,
                                      isInactive || players[0].inactive, counterJ, player0_has_initiative, ehp, ahp);
                }
                if (action == HEAL || action == MEDICINAL_HERBS) {
                    Player::heal(players[0], basedamage);

                    if (maxElement != -1){
                        if (values[exCounter++] != -1){
                            return false;
                        }
                        if(maxElement <= exCounter){
                            return true;
                        }
                    }

                } else {
                    Player::reduceHp(players[1], basedamage);

                    if (maxElement != -1){
                        if (basedamage != 0&&values[exCounter++] != basedamage){
                            return false;
                        }
                        if(maxElement <= exCounter){
                            return true;
                        }
                    }
                }
                //--------start_FUN_021594bc-------
                if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                    (*position) += 1;
                }
            }
            //--------end_FUN_021594bc-------
        }
        if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
            (*position) += 1;
        }
        camera::Main(position, actions);

#ifdef DEBUG2
        DEBUG_COUT2((*position));
        if ((*position) == 176){
            std::cout << "!!" << std::endl;
        }
#endif

        if (!Player::isPlayerAlive(players[1])) {
            return false;
        }
        if (!Player::isPlayerAlive(players[0])) {
            return false;
        }

        Player::heal(players[0], 25);

    }
    if(maxElement != -1){
        return false;
    }else{
        return true;
    }
}

double BattleEmulator::FUN_021dbc04(int baseHp, double maxHp) {
    auto hp = static_cast<double>(baseHp);
    if (hp == 0) {
        return 0;
    }
    return hp / maxHp;
}

//コンパイラが毎回コピーするコードを生成するからグローバルスコープに追い出しとく
const int proportionTable2[9] = {90, 90, 64, 32, 16, 8, 4, 2, 1};//最後の項目を調べるのは手動　P:\lua\isilyudaru\hissatuteki.lua
const int proportionTable3[9] = {63, 56, 49, 42, 35, 28, 21, 14, 7};//6ダメージ以下で0%
// double proportionTable1[9] = {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 3.0, 0.2, 0.1};// 21/70が2.99999...になるから最初から20/70より大きい2.89にしちゃう


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
    int baseDamage_tmp = -1;
    int OffensivePower = players[attacker].atk;
    double percent1 = 0.0;
    int percent_tmp;
    auto totalDamme = 0;
    switch (Id & 0xffff) {
        case BattleEmulator::BUFF:
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++; //0x021e54fc 不明
            if (!players[0].specialCharge){
                (*position)++;//0x021ed7a8 必殺チャージ(敵) 0%
                if (lcg::getPercent(position, 100) < 1) {//0x021edaf4
                    players[defender].specialCharge = true;
                    players[defender].specialChargeTurn = 6;
                }
            }
            if (players[0].BuffLevel != 2) {
                players[0].BuffLevel++;
                if (players[0].BuffLevel == 1) {
                    players[0].def = static_cast<int>(floor(players[0].defaultDEF * 1.5));
                }else if(players[0].BuffLevel == 2){
                    players[0].def = static_cast<int>(floor(players[0].defaultDEF * 2));
                }
                players[0].BuffTurns = 7;
            }
            break;
        case BattleEmulator::HIGH_SPEED_COMBO:
            (*position) += 2;
            (*position) += 4;
            for (int i = 0; i < 4; ++i) {
                (*position)++; // アクロバットスターとか

                (*position)++;//会心
                if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                    if (lcg::getPercent(position, 100) < 11) {// = 10%
                        kaihi = true;
                    }
                    if (!kaihi && lcg::getPercent(position, 100) < 11) {
                        tate = true;
                    }
                }
                (*position)++;//回避

                baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

                if (kaihi) {
                    if (!players[0].specialCharge) {
                        (*position)++;//0x021ed7a8
                    }
                    baseDamage = 0;
                } else if (tate) {
                    if (!players[0].specialCharge) {
                        (*position)++;//0x021ed7a8
                    }
                    baseDamage = 0;
                } else {
                    if (baseDamage == 0){
                        baseDamage = lcg::getPercent(position, 2);//TODO: 0x021e81a0
                    }

                    if (baseDamage != 0){
                        (*position)++;//目を覚ました
                        (*position)++;//不明
                    }
                    process7A8(position, baseDamage, players, defender);
                }
                tmp = static_cast<double>(baseDamage);
                tmp = tmp * players[defender].defence;
                baseDamage = static_cast<int>(floor(tmp));
                totalDamme += baseDamage;
            }
            return totalDamme;
        case BattleEmulator::MEDICINAL_HERBS:
            players[0].medicinal_herbs_count--;
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
        case BattleEmulator::DEFENCE:
        case BattleEmulator::CURE_PARALYSIS:
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
        case BattleEmulator::ATTACK_ENEMY:
        case BattleEmulator::SKY_ATTACK:
            (*position) += 2;
            (*position)++; // アクロバットスターとか

            (*position)++;//会心
            if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                if (lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi && lcg::getPercent(position, 100) < 11) {
                    tate = true;
                }
            }
            (*position)++;//回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            if (kaihi) {
                if (!players[0].specialCharge) {
                    (*position)++;//0x021ed7a8
                }
                baseDamage = 0;
            } else if (tate) {
                if (!players[0].specialCharge) {
                    (*position)++;//0x021ed7a8
                }
                baseDamage = 0;
            } else {
                if (baseDamage == 0){
                    baseDamage = lcg::getPercent(position, 1);//TODO: 0x021e81a0
                }

                if (baseDamage != 0){
                    (*position)++;//目を覚ました
                    (*position)++;//不明
                }
                process7A8(position, baseDamage, players, defender);
            }
            tmp = static_cast<double>(baseDamage);
            if ((Id & 0xffff) == BattleEmulator::SKY_ATTACK){
                tmp = tmp * 1.5;
            }
            tmp = tmp * players[defender].defence;
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
                if (!players[1].rage) {
                    (*position)++;//会心時特殊処理　0x021e54fc
                    (*position)++;//会心時特殊処理　0x021eb8c8
                } else {
                    (*position)++;//会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }
            }
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].mp -= 2;
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
                    double percent = FUN_021dbc04(preHP[1], players[1].maxHp);
                    if (percent >= 0.5) {
                        if (!players[1].rage) {
                            (*position)++;
                            players[1].rage = true;
                            players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                        } else {
                            (*position)++;
                        }
                    } else {
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
                if (!players[1].rage) {
                    (*position)++;//会心時特殊処理　0x021e54fc
                    (*position)++;//会心時特殊処理　0x021eb8c8
                } else {
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

void inline BattleEmulator::process7A8(int * position,int baseDamage,Player players[2], int defender){
    if (!players[defender].paralysis && !players[defender].inactive&&!players[defender].sleeping) {
        //必殺チャージ(敵)
        if (!players[defender].specialCharge) {
            auto percent_tmp = lcg::getPercent(position, 100);
            auto tmp = baseDamage * players[defender].defence;
            auto baseDamage_tmp = static_cast<int>(floor(tmp));
            for (int i = 0; i < 9; ++i) {
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

int BattleEmulator::ProcessEnemyRandomAction2A(int *position) {//0x0208aca8
    const int patternTable[6] = {43, 42, 43, 43, 42, 43};
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


int BattleEmulator::ProcessEnemyRandomAction44(int *position) {//0x0208aca8
    const int patternTable[6] = {68, 58, 48, 38, 27, 17};
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


