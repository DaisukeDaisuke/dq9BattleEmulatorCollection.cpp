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
#include "Equipment.h"


int32_t actions[3];
int actionsPosition = 0;
int preHP[3] = {0, 0, 0};
bool player0_has_initiative = false;
int previousAttack = -1;
int comboCounter = 0;


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
        case BattleEmulator::ULTRA_HIGH_SPEED_COMBO:
            return "High-Speed Combo";
        case BattleEmulator::SKY_ATTACK:
            return "Sky Attack";
        case BattleEmulator::CRITICAL_ATTACK:
            return "Critical Attack";
        case BattleEmulator::LAUGH:
            return "Laugh";
        case BattleEmulator::DISRUPTIVE_WAVE:
            return "Disruptive Wave";
        case BattleEmulator::BURNING_BREATH:
            return "Burning Breath";
        case BattleEmulator::DARK_BREATH:
            return "Dark Breath";
        case BattleEmulator::MORE_HEAL:
            return "More Heal(hoimu)";
        case BattleEmulator::MIDHEAL:
            return "Mid Heal(behoimi)";
        case BattleEmulator::FREEZING_BLIZZARD:
            return "freezing blizzard";
        case BattleEmulator::MERA_ZOMA:
            return "Mera Zoma";
        case BattleEmulator::DOUBLE_UP:
            return "Double up";
        case BattleEmulator::MULTITHRUST:
            return "Multithrust";

        case BattleEmulator::ATTACK_ALLY:
            return "Attack";
        case BattleEmulator::HEAL:
            return "Heal";
        case BattleEmulator::DEFENCE:
            return "Defence";

        case BattleEmulator::MAGIC_MIRROR:
            return "magic mirror";

        case BattleEmulator::LIGHTNING_STORM:
            return "Lightning Storm";
        case BattleEmulator::LULLAB_EYE:
            return "Lullab Eye";
        case BattleEmulator::SLEEPING:
            return "Sleeping";
        case BattleEmulator::FULLHEAL:
            return "Full heal(behoma)";
        case BattleEmulator::DEFENDING_CHAMPION:
            return "Defense Champion";//defending champion
        case BattleEmulator::PSYCHE_UP:
            return "Psyche up";

        default:
            return "Unknown Action";
    }
}

bool BattleEmulator::Main(int *position, int RunCount, std::vector<int32_t> Gene, Player *players,
                          std::optional<BattleResult> &result,
                          uint64_t seed, const int values[50], int maxElement, uint64_t *NowState) {
    resetCombo();
    player0_has_initiative = false;
    actionsPosition = 0;
    int doAction = -1; // デバッグ用
    int genePosition = 0;
    int exCounter = 0;
    uint64_t tmpState = -1;
    auto startPos = static_cast<int>(((*NowState) >> 24) & 0xffff);
    if (startPos != 0) {
        startPos++;
    }
    for (int counterJ = startPos; counterJ < RunCount; ++counterJ) {
        //現在ターンを保存
        (*NowState) &= ~0xffff000000;
        (*NowState) |= (counterJ << 24);

        if (players[0].dirtySpecialCharge) {
            players[0].specialCharge = false;
            players[0].dirtySpecialCharge = false;
        }
        players[0].specialChargeTurn--;
        if (players[0].specialChargeTurn == -1) {
            players[0].specialCharge = false;
        }

        resetCombo();

        tmpState = (*NowState);

#ifdef DEBUG2
        DEBUG_COUT2((*position));
        if ((*position) == 952) {
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
        int enemyAction[2] = {0, 0};
        while (counter != 2) {
            auto state = (*NowState) & 0xff;
            if (state == TYPE_2A) {
                //TODO: 2回同じ行動が選択された時の挙動を調べる
                const int table[6] = {ATTACK_ENEMY, ULTRA_HIGH_SPEED_COMBO, SWITCH_2B, SWITCH_2C, LIGHTNING_STORM,
                                      CRITICAL_ATTACK};
                enemyAction[counter] = table[ProcessEnemyRandomAction44(position)];
                if (counter == 1 && enemyAction[0] == enemyAction[1]) {
                    if (enemyAction[counter] == ULTRA_HIGH_SPEED_COMBO) {
                        enemyAction[counter] = ATTACK_ENEMY;
                    }
                }
                if (enemyAction[counter] == SWITCH_2B) {
                    (*NowState) &= ~0xffff; //ついでに2Eステートもクリアしとく
                    (*NowState) |= TYPE_2B;
                    (*position) += 2;
                    //TODO
                    continue;
                } else if (enemyAction[counter] == SWITCH_2C) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2C;
                    (*position) += 2;
                    //TODO
                    continue;
                } else if (enemyAction[counter] == ATTACK_ENEMY) {
                    (*position) += 3;
                } else if (enemyAction[counter] == ULTRA_HIGH_SPEED_COMBO) {
                    (*position) += 4;
                } else if (enemyAction[counter] == CRITICAL_ATTACK) {
                    (*position)++;
                } else if (enemyAction[counter] == LIGHTNING_STORM) {
                    (*position) += 2;
                }
            } else if (state == TYPE_2C) {
                const int table[6] = {SKY_ATTACK, MERA_ZOMA, FREEZING_BLIZZARD, SWITCH_2A, LULLAB_EYE, SWITCH_2E};
                enemyAction[counter] = table[ProcessEnemyRandomAction44(position)];
                if (counter == 1 && enemyAction[0] == enemyAction[1]) {
                    if (enemyAction[counter] == FREEZING_BLIZZARD) {
                        enemyAction[counter] = MERA_ZOMA;
                    } else if (enemyAction[counter] == SKY_ATTACK) {
                        enemyAction[counter] = MERA_ZOMA;
                    } else if (enemyAction[counter] == MERA_ZOMA) {
                        enemyAction[counter] = SKY_ATTACK;
                    }
                }
                if (enemyAction[counter] == SWITCH_2A) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2A;
                    (*position) += 2;
                    continue;
                } else if (enemyAction[counter] == SWITCH_2E) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2E;
                    (*position) += 2;
                    continue;
                } else if (enemyAction[counter] == SKY_ATTACK) {
                    (*position)++;
                } else if (enemyAction[counter] == MERA_ZOMA) {
                    (*position)++;//攻撃先決定
                } else if (enemyAction[counter] == LULLAB_EYE) {
                    if (players[0].sleeping) {
                        std::cout << "!!" << std::endl;//TODO
                    }
                    (*position)++;//攻撃先決定 0x021ee074
                }
            } else if (state == TYPE_2B) {
                const int attack[6] = {LAUGH, DISRUPTIVE_WAVE, BURNING_BREATH, DARK_BREATH, SWITCH_2C, SWITCH_2D};
                enemyAction[counter] = attack[FUN_0208aecc(position, NowState)];
                if (enemyAction[counter] == SWITCH_2C) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2C;
                    (*position) += 2;
                    continue;
                } else if (enemyAction[counter] == SWITCH_2D) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2D;
                    (*position) += 2;
                    continue;
                } else if (enemyAction[counter] == LAUGH) {
                    (*position) += 2;
                } else if (enemyAction[counter] == DISRUPTIVE_WAVE) {
                    (*position) += 2;
                }
            } else if(state == TYPE_2D){
                const int attack[6] = {PSYCHE_UP, ULTRA_HIGH_SPEED_COMBO, ATTACK_ENEMY, SKY_ATTACK, SWITCH_2A, SWITCH_2B};
                enemyAction[counter] = attack[FUN_0208aecc(position, NowState)];
                if (enemyAction[counter] == SWITCH_2A) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2A;
                    (*position) += 2;
                    continue;
                } else if (enemyAction[counter] == SWITCH_2B) {
                    (*NowState) &= ~0xffff;
                    (*NowState) |= TYPE_2B;
                    (*position) += 2;
                    continue;
                }

                if (enemyAction[counter] == ULTRA_HIGH_SPEED_COMBO) {
                    (*position) += 4;
                }else if (enemyAction[counter] == SKY_ATTACK) {
                    (*position)++;
                }else if (enemyAction[counter] == ATTACK_ENEMY) {
                    (*position) += 3;
                }
            }
            if (counter == 0) {
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

        //途中で解除してもいいように2回チェックする
        if (players[0].sleeping) {
            actionTable = SLEEPING;
        }

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
                for (const int c: enemyAction) {
                    //--------start_FUN_02158dfc-------
                    (*position)++;
                    //--------end_FUN_02158dfc-------
                    basedamage = callAttackFun(c, position, players, 1, 0);

                    if (maxElement != -1) {
                        if (basedamage != 0 && values[exCounter++] != basedamage) {
                            return false;
                        }
                        if (maxElement <= exCounter) {
                            return true;
                        }
                    } else {
                        BattleResult::add(result, c, basedamage, true, players[0].AtkBuffTurn,
                                          players[0].BuffTurns, players[0].MagicMirrorTurn, counterJ,
                                          player0_has_initiative, ehp,
                                          ahp, tmpState);
                    }
                    if (c == BattleEmulator::MERA_ZOMA && players[0].hasMagicMirror) {
                        Player::reduceHp(players[1], basedamage);
                    } else {
                        Player::reduceHp(players[0], basedamage);
                    }
                    //--------start_FUN_021594bc-------
                    if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                        (*position) += 1;
                    }
                    //--------end_FUN_021594bc-------
                }
            } else {
                int32_t action = actionTable & 0xffff;

                //--------start_FUN_02158dfc-------
                if (!players[0].paralysis && !players[0].sleeping) {
                    (*position) += 1;
                } else if (players[0].paralysis) {
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
                } else if (players[0].sleeping) {
                    action = SLEEPING;

                    players[0].sleepingTurn--;
                    if (players[0].sleepingTurn <= 0) {
                        const int sleepTable[4] = {37, 62, 87, 100};
                        auto probability1 = sleepTable[std::abs(players[0].sleepingTurn)];
                        auto probability2 = lcg::getPercent(position, 100);
                        if (probability1 >= probability2) {
                            players[0].sleeping = false;
                        }
                    } else {
                        (*position)++;
                    }

                }

                doAction = action;


                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(action, position, players, 0, 1);
                if (maxElement == -1) {
                    BattleResult::add(result, action, basedamage, false, players[0].AtkBuffTurn,
                                      players[0].BuffTurns, players[0].MagicMirrorTurn, counterJ,
                                      player0_has_initiative, ehp, ahp,
                                      tmpState);
                }
                if (action == HEAL || action == MEDICINAL_HERBS || action == MORE_HEAL || action == MIDHEAL ||
                    action == FULLHEAL) {
                    Player::heal(players[0], basedamage);

                    if (maxElement != -1) {
                        if (values[exCounter++] != -1) {
                            return false;
                        }
                        if (maxElement <= exCounter) {
                            return true;
                        }
                    }

                } else {
                    Player::reduceHp(players[1], basedamage);

                    if (maxElement != -1) {
                        if (basedamage != 0 && values[exCounter++] != basedamage) {
                            return false;
                        }
                        if (maxElement <= exCounter) {
                            return true;
                        }
                    }
                }
                //--------start_FUN_021594bc-------
                if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                    (*position) += 1;
                }

                players[0].MagicMirrorTurn--;
                if (players[0].hasMagicMirror && players[0].MagicMirrorTurn <= 0) {
                    const int probability[4] = {62, 75, 87, 100};
                    auto probability1 = probability[std::abs(players[0].MagicMirrorTurn)];
                    auto probability2 = lcg::getPercent(position, 100);
                    if (probability1 >= probability2) { // 0x0215a050
                        players[0].hasMagicMirror = false;
                    }
                }
                players[0].BuffTurns--;
                if (players[0].BuffLevel != 0 && players[0].BuffTurns == 0) {
                    const int probability[4] = {62, 75, 87, 100};
                    auto probability1 = probability[std::abs(players[0].BuffTurns)];
                    auto probability2 = lcg::getPercent(position, 100);
                    if (probability1 >= probability2) {
                        players[0].BuffLevel = 0;
                        RecalculateBuff(players);
                    }
                }
                players[0].AtkBuffTurn--;
                if (players[0].AtkBuffLevel != 0 && players[0].AtkBuffTurn == 0) {//0x0215a804
                    const int probability[4] = {62, 75, 87, 100};
                    auto probability1 = probability[std::abs(players[0].AtkBuffTurn)];
                    auto probability2 = lcg::getPercent(position, 100);
                    if (probability1 >= probability2) {
                        players[0].AtkBuffLevel = 0;
                        RecalculateBuff(players);
                    }
                }


            }
            //--------end_FUN_021594bc-------
        }
        if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
            (*position) += 1;
        }
        camera::Main(position, actions, NowState, player0_has_initiative);

#ifdef DEBUG2
        DEBUG_COUT2((*position));
        if ((*position) == 176) {
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
    if (maxElement != -1) {
        return false;
    } else {
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
const int proportionTable3[9] = {279, 248, 217, 186, 155, 124, 93, 62, 31};//6ダメージ以下で0% 309
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
    auto totalDamage = 0;
    auto attackCount = 0;
    switch (Id & 0xffff) {
        case DARK_BREATH:
            (*position) += 2;
            (*position)++;//会心
            (*position)++;//不明
            if (lcg::getPercent(position, 100) < 2) {
                kaihi = true;
            }
            (*position)++;//ニセ回避 0x02157f58
            baseDamage = FUN_021e8458_typeD(position, 10 ,65);
            tmp = baseDamage * Equipments::calculateTotalResistance(Attribute::Darkness);
            baseDamage = static_cast<int>(floor(tmp));
            if (!kaihi) {
                (*position)++;//0x021e54fc 不明
            } else {
                baseDamage = 0;
            }
            process7A8(position, baseDamage, players, defender);
            break;
        case PSYCHE_UP:
            (*position) += 2;
            break;
        case FULLHEAL:
            (*position) += 2;
            (*position)++;//不明　0x021ec6f8
            (*position)++;//会心
            (*position)++;//ニセ回避 0x02157f58
            FUN_0207564c(position, players[attacker].atk, players[attacker].def);
            (*position)++;//関係ない
            //0x021eb8c8, randIntRange: 0x021eb8f0 怒り狂っている場合←の消費が発生しない。
            //TODO 必殺チャージ
            if (!players[1].rage) {
                (*position)++;
            }
            (*position)++;//?
            if (!players[0].paralysis && !players[0].inactive) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].mp -= 28;
            resetCombo();
            return 999;
            break;
        case DISRUPTIVE_WAVE:
            //TODO: 必殺チャージ無しのときに7a8があるかどうか調べる
            (*position) += 2;
            (*position)++; //0x021ec6f8 会心
            (*position)++; //0x021ec6f8 不明
            (*position)++;//ニセ回避 0x02157f58
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明 0x021e54fc

            players[0].hasMagicMirror = false;
            players[0].MagicMirrorTurn = -1;
            players[0].AtkBuffLevel = 0;
            players[0].AtkBuffTurn = -1;
            players[0].BuffLevel = 0;
            players[0].BuffTurns = -1;


            RecalculateBuff(players);
            baseDamage = 0;
            resetCombo();
            break;
        case MIDHEAL:
            (*position) += 2;
            (*position)++; //0x021ec6f8 不明
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//回避

            baseDamage = FUN_021e8458_typeD(position, 10, CalculateMidHealBase(players));
            if (kaisinn) {
                tmp *= lcg::floatRand(position, 1.5, 2.0);//TODO
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
            players[attacker].mp -= 4;
            resetCombo();

            break;
        case LULLAB_EYE:
            (*position) += 2;
            (*position)++; //0x021ec6f8 不明
            (*position)++; //会心 0x02158584
            (*position)++;//ニセ回避 0x02157f58
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明 0x021e54fc
            players[0].sleeping = true;
            players[0].sleepingTurn = 2;
            baseDamage = 0;
            resetCombo();
            break;
        case LIGHTNING_STORM:
            (*position) += 2;
            (*position)++; //会心 0x02158584
            (*position)++; //0x021ec6f8 不明
            if (!players[0].paralysis && !players[0].sleeping) {//TODO
                (*position)++;//盾ガード 0x021586fc 0%
            }
            (*position)++;//ニセ回避 0x02157f58 100%
            baseDamage = FUN_021e8458_typeD(position, 15, 80);
            tmp = baseDamage * Equipments::calculateTotalResistance(Attribute::ThunderExplosion);
            baseDamage = static_cast<int>(floor(tmp));
            (*position)++;//?? 0x02158ac4
            (*position)++;//?? 0x021e54fc
            process7A8(position, baseDamage, players, defender);
            resetCombo();
            if (baseDamage != 0 && players[0].sleeping) {
                players[0].sleeping = false;
                players[0].sleepingTurn = -1;
            }
            resetCombo();
            break;
        case MULTITHRUST:
            attackCount = lcg::intRangeRand(position, 3, 4);
            (*position)++;
            (*position) += attackCount;
            for (int i = 0; i < attackCount; ++i) {
                kaihi = false;
                tate = false;
                (*position)++; //0x021ec6f8 不明
                if (lcg::getPercent(position, 0x2710) < 83) {
                    kaisinn = true;
                }
                if (lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                (*position)++;//盾ガード 0x021586fc 0%
                (*position)++;//ニセ回避 0x02157f58 100%
                baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
                tmp = floor(baseDamage * 0.5) * 1.25;//1.25倍は雷属性になってるから
                baseDamage = static_cast<int>(tmp);
                if (kaisinn) {//0x020759ec
                    tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                    baseDamage = static_cast<int>(floor(tmp));
                }

                if (!kaihi) {
                    ProcessRage(position, baseDamage, preHP, players);
                    (*position)++;//目を覚ました
                    (*position)++;//不明 0x021e54fc
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
                preHP[1] = std::max(0, preHP[1] - baseDamage);
                totalDamage += baseDamage;
                if (preHP[1] <= 0) {
                    return totalDamage;
                }
            }
            if (players[defender].hp - baseDamage >= 0) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            resetCombo();
            return totalDamage;
        case MERA_ZOMA:
            (*position) += 2;
            (*position)++; //0x021ec6f8 不明
            (*position)++; //敵の会心判定
            if (players[0].hasMagicMirror) {
                if (lcg::getPercent(position, 0x2710) < 100) { //こっちの会心判定
                    kaisinn = true;
                }
                (*position)++;//盾ガード 0x021586fc 0%
                (*position)++;//ニセ回避 0x02157f58 100%
                tmp = BattleEmulator::FUN_021e8458_typeD(position, 12, 190);
                if (kaisinn) {
                    tmp *= lcg::floatRand(position, 1.5, 2.0);
                }
                tmp *= 1.25;
                baseDamage = static_cast<int>(floor(tmp));
                (*position)++;//不明 0x021e54fc
            } else {
                if (!players[0].paralysis && !players[0].sleeping) {
                    (*position)++;//盾ガード 0x021586fc 0%
                }
                (*position)++;//ニセ回避 0x02157f58 100%
                baseDamage = FUN_021e8458_typeD(position, 12, 116);
                tmp = baseDamage * Equipments::calculateTotalResistance(Attribute::Fire);
                baseDamage = static_cast<int>(floor(tmp));
                (*position)++;//0x021e54fc 不明
                process7A8(position, baseDamage, players, defender);//必殺チャージ(敵)　0x021ed7a8
            }
            resetCombo();
            break;
        case BattleEmulator::FREEZING_BLIZZARD:
            (*position) += 2;
            (*position)++; // 会心判定
            (*position)++; //0x021ec6f8 不明
            if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                if (lcg::getPercent(position, 100) < 2) { //0x021587b0 本物みかわし
                    kaihi = true;
                }
            }
            (*position)++; // 0x02157f58 ニセ回避

            baseDamage = FUN_021e8458_typeD(position, 15, 75);
            baseDamage = static_cast<int>(floor(baseDamage * Equipments::calculateTotalResistance(Attribute::Ice)));

            if (!kaihi) {
                (*position)++;//0x021e54fc 不明
            } else {
                baseDamage = 0;
            }
            process7A8(position, baseDamage, players, defender);
            resetCombo();
            break;
        case BattleEmulator::DOUBLE_UP:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[attacker].def);
            (*position)++;//かぶとわりの判定　0x021e3e7c
            (*position)++;//なんか　0x021e54fc
            if (!players[attacker].specialCharge) {
                (*position)++;//必殺(敵)　0x021ed7a8
                if (!players[0].paralysis && !players[0].inactive) {
                    if (lcg::getPercent(position, 100) < 1) {
                        players[attacker].specialCharge = true;
                        players[attacker].specialChargeTurn = 6;
                    }
                }
            }

            if (players[0].AtkBuffLevel != 2) {
                players[0].AtkBuffLevel += 2;
                players[0].AtkBuffLevel = std::min(players[0].AtkBuffLevel, 2);
                players[0].AtkBuffTurn = 6;
            }

            if (players[0].BuffLevel != -2) {
                players[0].BuffLevel--;
                players[0].BuffTurns = 7;
            }

            RecalculateBuff(players);
            resetCombo();
            break;
        case BattleEmulator::MORE_HEAL:
            (*position) += 2;
            (*position)++;//関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++;//回避

            baseDamage = FUN_021e8458_typeD(position, 20, CalculateMoreHealBase(players));
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);//TODO
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
            players[attacker].mp -= 8;
            resetCombo();
            break;
        case BattleEmulator::MAGIC_MIRROR:
            (*position) += 5;
            FUN_0207564c(position, players[attacker].atk, players[attacker].def);
            (*position)++;//不明
            if (!players[0].specialCharge) {
                (*position)++;//0x021ed7a8
                if (lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[0].hasMagicMirror = true;
            players[0].MagicMirrorTurn = 6;
            resetCombo();
            break;
        case BattleEmulator::CRITICAL_ATTACK:
            (*position) += 2;
            (*position)++; // アクロバットスターとか

            if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                if (lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi && lcg::getPercent(position, 100) < 11) {
                    tate = true;
                }
            }
            (*position)++;//回避

            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            baseDamage = static_cast<int>(floor(players[1].defaultATK * lcg::floatRand(position, 0.8500, 0.9500)));

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
                if (baseDamage == 0) {
                    baseDamage = lcg::getPercent(position, 1);//TODO: 0x021e81a0
                }

                if (baseDamage != 0) {
                    (*position)++;//目を覚ました
                    (*position)++;//不明
                }
                process7A8(position, baseDamage, players, defender);
            }
            tmp = baseDamage * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            resetCombo();
            break;
        case BattleEmulator::BUFF:
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            FUN_0207564c(position, players[attacker].atk, players[attacker].def);
            (*position)++; //0x021e54fc 不明
            if (!players[0].specialCharge) {
                (*position)++;//0x021ed7a8 必殺チャージ(敵) 0%
                if (lcg::getPercent(position, 100) < 1) {//0x021edaf4
                    players[defender].specialCharge = true;
                    players[defender].specialChargeTurn = 6;
                }
            }
            if (players[0].BuffLevel != 2) {
                players[0].BuffLevel++;
                players[0].BuffTurns = 7;
                RecalculateBuff(players);
            }
            resetCombo();
            break;
        case BattleEmulator::ULTRA_HIGH_SPEED_COMBO:
            (*position) += 2;
            (*position) += 4;
            for (int i = 0; i < 4; ++i) {
                kaihi = false;
                tate = false;
                (*position)++; // アクロバットスターとか

                (*position)++;//会心
                if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                    if (lcg::getPercent(position, 100) < 2) {// = 10%
                        kaihi = true;
                    }
                    if (!kaihi && lcg::getPercent(position, 100) < 11) {//TODO 盾の条件調べる
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
                    tmp = baseDamage * 0.3;
                    baseDamage = static_cast<int>(floor(tmp));
                    if (baseDamage == 0) {
                        baseDamage = lcg::getPercent(position, 2);//TODO: 0x021e81a0
                    }

                    if (baseDamage != 0) {
                        (*position)++;//目を覚ました
                        (*position)++;//不明
                    }
                    process7A8(position, baseDamage, players, defender);
                }
                tmp = static_cast<double>(baseDamage);
                tmp = tmp * players[defender].defence;
                baseDamage = static_cast<int>(floor(tmp));
                totalDamage += baseDamage;
            }
            if (totalDamage != 0 && players[0].sleeping) {
                players[0].sleeping = false;
                players[0].sleepingTurn = -1;
            }
            resetCombo();
            return totalDamage;
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
            resetCombo();
            break;
        case BattleEmulator::SLEEPING:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            (*position)++;//不明
            baseDamage = 0;
            resetCombo();
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
            resetCombo();
            break;
        case BattleEmulator::LAUGH:
            (*position) += 2;
            (*position)++;//関係ない
            (*position)++;//会心
            (*position)++;//回避
            FUN_0207564c(position, players[attacker].atk, players[attacker].def);
            (*position)++;//不明
            baseDamage = 0;
            resetCombo();
            break;
        case BattleEmulator::BURNING_BREATH:
            (*position) += 2;
            (*position)++;//会心
            (*position)++;//関係ない
            if (lcg::getPercent(position, 100) < 2) {//0x021587b0 回避
                kaihi = true;
            }
            if (lcg::getPercent(position, 100) < 13 && !kaihi) {
                if (players[defender].paralysisLevel == 3) {
                    //std::cerr << "paralysisLevel == 2" << std::endl;
                }
                players[defender].paralysis = true;
                players[defender].paralysisTurns = 4;
                players[defender].paralysisLevel++;
                players[0].sleeping = false;
                players[0].sleepingTurn = -1;
                FUN_0207564c(position, players[attacker].atk, players[defender].def);
            }
            if (!players[0].specialCharge) {
                (*position)++;//0x021ed7a8 必殺(敵)
            }
            baseDamage = 0;
            resetCombo();
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
                if (baseDamage == 0) {
                    baseDamage = lcg::getPercent(position, 2);//TODO: 0x021e81a0
                }

                if (baseDamage != 0) {
                    (*position)++;//目を覚ました
                    (*position)++;//不明
                }
                process7A8(position, baseDamage, players, defender);
            }
            tmp = static_cast<double>(baseDamage);
            if ((Id & 0xffff) == BattleEmulator::SKY_ATTACK) {
                tmp = tmp * 1.5;
            }

            tmp = processCombo(Id & 0xffff, tmp);

            tmp = tmp * players[defender].defence;
            baseDamage = static_cast<int>(floor(tmp));
            if (baseDamage != 0 && players[0].sleeping) {
                players[0].sleeping = false;
                players[0].sleepingTurn = -1;
            }
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
            baseDamage = 0;
            resetCombo();
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
            if (!players[0].paralysis && !players[0].inactive && !players[0].sleeping) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].mp -= 2;
            resetCombo();
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
                ProcessRage(position, baseDamage, preHP, players);
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

void BattleEmulator::process7A8(int *position, int baseDamage, Player players[2], int defender) {
    if (!players[defender].paralysis && !players[defender].inactive && !players[defender].sleeping) {
        if (players[defender].hp > baseDamage) {//hpが0以下の場合必殺チャージの判定は発生しない。
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
    } else if (players[defender].sleeping) {
        (*position)++;
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

int BattleEmulator::FUN_0208aecc(int *position, uint64_t *NowState) {
    auto previousState = ((*NowState) >> 8) & 0xff;
    if (previousState == 3) {
        previousState = 0;
    }
    uint64_t r0_var2 = lcg::getSeed(position);
    uint64_t r3_var3 = previousState;
    uint64_t r2_var5 = r0_var2 & 0x1;
    uint64_t r0_var6 = r3_var3 << 0x1 & 0xFFFFFFFF;
    uint64_t r1_var7 = r0_var6 & 0xff;
    uint64_t r0_var8 = r3_var3 + 0x1;
    uint64_t r3_var9 = r1_var7 + r2_var5;
    previousState = static_cast<int>(r0_var8);
    uint64_t r3_var12 = r3_var9 & 0xff;
    DEBUG_COUT2(previousState);
    (*NowState) &= ~0xff00;
    (*NowState) |= (previousState << 8);
    return static_cast<int>(r3_var12);
}

int BattleEmulator::CalculateMoreHealBase(Player *players) {//ベホイミ
    double tmp1 = (players[0].HealPower - 200) * 0.5194;
    auto tmp2 = static_cast<int>(floor(tmp1));
    return 185 + tmp2;
}

int BattleEmulator::CalculateMidHealBase(Player *players) {//ｂ
    double tmp1 = (players[0].HealPower - 100) * 0.2392;
    auto tmp2 = static_cast<int>(floor(tmp1));
    return 85 + tmp2;
}

void BattleEmulator::RecalculateBuff(Player *players) {
    // 定数の倍率を格納した配列
    const double ATKMultipliers[] = {0.5, 0.75, 1.0, 1.25, 1.5};
    const double DEFMultipliers[] = {0.25, 0.5, 1.0, 1.5, 2.0};

    // BuffLevel に +2 して配列インデックスに変換
    int index = players[0].BuffLevel + 2;

    // インデックスが範囲外でないかチェック（-2 <= BuffLevel <= 2 の範囲であることを確認）
    if (index >= 0 && index < 5) {
        players[0].def = static_cast<int>(floor(players[0].defaultDEF * DEFMultipliers[index]));
    }

    int index1 = players[0].AtkBuffLevel + 2;
    if (index >= 0 && index < 5) {
        players[0].atk = static_cast<int>(floor(players[0].defaultATK * ATKMultipliers[index1]));
    }
}

void BattleEmulator::ProcessRage(int *position, int baseDamage, int *preHP, Player *players) {
    auto percent1 = FUN_021dbc04(preHP[1] - baseDamage, players[1].maxHp);
    if (percent1 < 0.5) {
        double percent = FUN_021dbc04(preHP[1], players[1].maxHp);
        if (percent >= 0.5) {
            if (!players[1].rage) {
                (*position)++;
                //players[1].rage = true;
                //players[1].rageTurns = lcg::intRangeRand(position, 2, 4);
                (*position)++;
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
}

