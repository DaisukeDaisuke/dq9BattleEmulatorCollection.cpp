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

const char *BattleEmulator::getActionName(int actionId) {
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
        case BattleEmulator::CURE_SLEEPING:
            return "Cure Sleeping";

        case BattleEmulator::FULLHEAL:
            return "Full heal(behoma)";
        case BattleEmulator::DEFENDING_CHAMPION:
            return "Defense Champion"; //defending champion
        case BattleEmulator::PSYCHE_UP:
            return "Psyche up";
        case BattleEmulator::MEDITATION:
            return "Meditation";
        case BattleEmulator::MAGIC_BURST:
            return "magic Burst";
        case BattleEmulator::RESTORE_MP:
            return "Restore MP";
        case BattleEmulator::MERCURIAL_THRUST:
            return "Mercurial Thrust";
        case BattleEmulator::TURN_SKIPPED:
            return "**Turn Skipped**";

        case BattleEmulator::SAGE_ELIXIR:
            return "Sage Elixir";
        case BattleEmulator::ELFIN_ELIXIR:
            return "Elfin Elixir";
        case BattleEmulator::MAGIC_WATER:
            return "Magic Water";
        case BattleEmulator::GOSPEL_SONG:
            return "gospel song";
        case BattleEmulator::FLEE_ALLY:
            return "Flee";
        case BattleEmulator::POISON_ATTACK:
            return "Poison Attack";
        case BattleEmulator::KASAP:
            return "Kasap(rukanann)";
        case BattleEmulator::SPECIAL_MEDICINE:
            return "Special Medicine";
        case BattleEmulator::MIRACLE_SLASH:
            return "Miracle Slash";
        case BattleEmulator::SWEET_BREATH:
            return "Sweet Breath";
        case BattleEmulator::DECELERATLE:
            return "Deceleratle(bomi)";
        case BattleEmulator::SPECIAL_ANTIDOTE:
            return "!Special Antidote";
        case BattleEmulator::ACROBATIC_STAR:
            return "Acrobatic Star";
        case BattleEmulator::CRACKLE:
            return "Crackle";
        case BattleEmulator::TIDAL_WAVE:
            return "Tidal Wave";
        case BattleEmulator::MASSIVE_SWIPE:
            return "Massive Swipe";
        case BattleEmulator::INACTIVE_ENEMY:
            return "Inactive";
        default:
            return "Unknown Action";
    }
}

thread_local int threadTurnProcessed = 0;

void BattleEmulator::ResetTurnProcessed() {
    threadTurnProcessed = 0;
}

int BattleEmulator::getTurnProcessed() {
    return threadTurnProcessed;
}

inline void BattleEmulator::processTurn() {
    // ここでturnProcessedをインクリメントする処理を追加
    threadTurnProcessed++;
}

//#endif


bool BattleEmulator::Main(int *position, int RunCount, const int32_t Gene[350], Player *players,
                          std::optional<BattleResult> &result,
                          uint64_t seed, const int eActions[350], const int damages[350], int mode,
                          uint64_t *NowState) {
    bool player0_has_initiative = false;
    int genePosition = 0;
    int exCounter = 0;
    int exCounter1 = 0;
    uint64_t tmpState = -1;

    auto startPos = static_cast<int>(((*NowState) >> 12) & 0xfffff);
    if (startPos != 0) {
        startPos++;
        RunCount += startPos;
    } else {
        startPos = 1;
        RunCount++;
    }


    for (int counterJ = startPos; counterJ < RunCount; ++counterJ) {
        processTurn();
        if (genePosition != -1) {
            genePosition = counterJ - 1;
        }
        //現在ターンを保存
        (*NowState) &= ~0xFFFFF000;
        (*NowState) |= (counterJ << 12);

        players[0].specialChargeTurn--;
        if (players[0].specialChargeTurn == -1) {
            players[0].specialCharge = false;
        }

        tmpState = (*NowState);

#ifdef DEBUG2
        std::cout << "c: " << counterJ << ", " << (*position) << std::endl;
        if ((*position) == 537) {
            std::cout << "!!" << std::endl;
        }
#endif
        int ehp = players[1].hp;
        int ahp = players[0].hp;

        players[0].defence = 1.0;

        int32_t actions[3] = {0, 0, 0};
        int actionsPosition = 0;

        (*position) += 2;
        player0_has_initiative = true;

        int enemyAction = ProcessEnemyRandomAction44(position);
        int32_t actionTable = -1;

        if (enemyAction == ATTACK_ENEMY) {
            if (!players[0].rage) {
                (*position)++; //0x02156874 0x00000002 4 攻撃先
            }
            (*position) += 2;
        }
        if (enemyAction == TIDAL_WAVE || enemyAction == MASSIVE_SWIPE) {
            (*position) += 2;
        }

        (*position)++; //0x02160d64

        if (Gene[genePosition] == 0 || Gene[genePosition] == -1) {
            genePosition = -1;
            //throw std::invalid_argument("GenePosition is invalid");
        }
        if (genePosition != -1 && Gene[genePosition] != 0 && Gene[genePosition] != -1) {
            actionTable = Gene[genePosition];
        } else {
            actionTable = ATTACK_ALLY;
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
            if (t == 1) {
                auto c = enemyAction;
                //--------start_FUN_02158dfc-------
                if (lcg::getPercent(position, 100) < 0.0330) {
                    //0x021588ec
                    //次の乱数が90%以上(一致含む)なら見惚れないらしい。
                    int mitore = lcg::getPercent(position, 100); //0x02158964
                    if (mitore < 90) {
                        c = INACTIVE_ENEMY;
                    }
                }
                if (players[0].rage) {
                    players[0].rageTurns--;
                    if (players[0].rageTurns <= 0) {
                        players[0].rage = false;
                    }
                }
                (*position)++;
                //--------end_FUN_02158dfc-------
                basedamage = callAttackFun(c, position, players, 1, 0, NowState);
                actions[actionsPosition++] = c;

                if (mode == -1) {
                    BattleResult::add(result, c, basedamage, true,
                                      counterJ - 1,
                                      player0_has_initiative, ehp,
                                      ahp, tmpState, players[0].specialChargeTurn, players[0].mp);
                } else if (mode != -1 && mode != -2) {
                    if (
                        c == ATTACK_ENEMY ||
                        c == POISON_ATTACK ||
                        c == KASAP ||
                        c == DECELERATLE ||
                        c == SWEET_BREATH
                    ) {
                        if (damages[exCounter] == -1) {
                            return true;
                        }
                        if (c == KASAP) {
                            if (damages[exCounter++] != -3) {
                                return false;
                            }
                        } else if (c == DECELERATLE) {
                            if (damages[exCounter++] != -2) {
                                return false;
                            }
                        } else if (c == SWEET_BREATH) {
                            if (damages[exCounter++] != -4) {
                                return false;
                            }
                        } else if (damages[exCounter] >= 0) {
                            if (damages[exCounter++] != basedamage) {
                                return false;
                            }
                        }
                    }
                }

                Player::reduceHp(players[0], basedamage);

                //--------start_FUN_021594bc-------
                if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                    (*position) += 1;
                } else {
                    break;
                }
                //--------end_FUN_021594bc-------
            } else {
                int32_t action = actionTable & 0xffff;
                auto skipTurn = false;
                if (action == BattleEmulator::FLEE_ALLY) {
                    skipTurn = true;
                }
                if (!skipTurn) {
                    //--------start_FUN_02158dfc-------
                    if (!players[0].paralysis && !players[0].sleeping) {
                        (*position) += 1;
                    } else if (players[0].sleeping) {
                        action = SLEEPING;

                        players[0].sleepingTurn--;
                        if (players[0].sleepingTurn <= 0) {
                            const int sleepTable[4] = {37, 62, 87, 100};
                            auto probability1 = sleepTable[std::abs(players[0].sleepingTurn)];
                            auto probability2 = lcg::getPercent(position, 100);
                            if (probability1 >= probability2) {
                                players[0].sleeping = false;
                                players[0].sleepingTurn = 0;
                                action = CURE_SLEEPING;
                            }
                        } else {
                            (*position)++;
                        }
                    }


                    //--------end_FUN_02158dfc-------
                    basedamage = callAttackFun(action, position, players, 0, 1, NowState);
                    actions[actionsPosition++] = action;

                    if (mode == -1) {
                        BattleResult::add(result, action, basedamage, false, counterJ - 1,
                                          player0_has_initiative, ehp, ahp,
                                          tmpState, players[0].specialChargeTurn, players[0].mp);
                    }
                    if (action == HEAL || action == MEDICINAL_HERBS || action == MORE_HEAL || action == MIDHEAL ||
                        action == FULLHEAL || action == SPECIAL_MEDICINE || action == GOSPEL_SONG || action ==
                        SPECIAL_ANTIDOTE) {
                        Player::heal(players[0], basedamage);
                        if (action == SPECIAL_MEDICINE || action == SPECIAL_ANTIDOTE) {
                            if (mode != -1 && mode != -2) {
                                if (damages[exCounter] == -5) {
                                    exCounter++;
                                }
                            }
                        }
                    } else {
                        Player::reduceHp(players[1], basedamage);

                        if (mode != -1 && mode != -2) {
                            if (action == ATTACK_ALLY || action == MIRACLE_SLASH) {
                                if (damages[exCounter] == -1) {
                                    return true;
                                }
                                //int need = ;
                                if (damages[exCounter++] != basedamage) {
                                    return false;
                                }
                            }
                        }
                    }
                    //--------start_FUN_021594bc-------
                    if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
                        (*position) += 1;
                        players[0].acrobaticStarTurn--;

                        if (players[0].acrobaticStar && players[0].acrobaticStarTurn == 0) {
                            players[0].acrobaticStar = false;
                            (*position)++; //0x0215aed4 チートで100にすると次のターンから判定が行われず永遠にアクロバットスター状態になる。
                            //3f800000
                            //3f600000
                            //3f200000
                            //3ec00000
                        }
                    }
                } else {
                    if (mode == -1) {
                        BattleResult::add(result, action, 0, false, counterJ - 1,
                                          player0_has_initiative, ehp, ahp,
                                          tmpState, players[0].specialChargeTurn, players[0].mp);
                    }
                }
            }
        }
        //--------end_FUN_021594bc-------

        if (Player::isPlayerAlive(players[0]) && Player::isPlayerAlive(players[1])) {
            (*position) += 1;
        }
        camera::Main(position, actions, NowState, player0_has_initiative, false);

#ifdef DEBUG2
        //DEBUG_COUT2((*position));
        if ((*position) == 176) {
            //std::cout << "!!" << std::endl;
        }
#endif

        if (!Player::isPlayerAlive(players[1])) {
            return false;
        }
        if (!Player::isPlayerAlive(players[0])) {
            return false;
        }
    }

    if (mode != -1 && mode != -2) {
        return true;
    } else {
        return false;
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
const int proportionTable2[9] = {90, 90, 64, 32, 16, 8, 4, 2, 1}; //最後の項目を調べるのは手動　P:\lua\isilyudaru\hissatuteki.lua
const int proportionTable3[9] = {101, 90, 79, 68, 57, 45, 34, 23, 12}; //6ダメージ以下で0% 309 new: 112 103
// double proportionTable1[9] = {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 3.0, 0.2, 0.1};// 21/70が2.99999...になるから最初から20/70より大きい2.89にしちゃう
const double Enemy_TensionTable[4] = {1.3, 2.0, 3.0, 4.5}; //一部の敵は特殊テンションテーブルを倍率として使う

/*
<?php
$base = 103;
$results = [];

for ($i = 9; $i >= 1; $i--) {
    $multiplier = $i / 10;
    $result = floor($base * $multiplier) + 1;
    $results[] = $result;
}

echo implode(", ", $results);
?>
*/


int BattleEmulator::callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender,
                                  uint64_t *NowState) {
    int baseDamage = 0;
    double tmp = 0;
    bool kaisinn = false;
    bool hasKaisinn = false;
    bool kaihi = false;
    bool tate = false;
    int OffensivePower = players[attacker].defaultATK;
    int percent_tmp;
    auto totalDamage = 0;
    auto attackCount = 0;
    bool defenseFlag = false; //防御した場合0x021e81a0のほうが優先度高いらしい。なんで
    switch (Id & 0xffff) {
        case TIDAL_WAVE:
            (*position) += 2;
            (*position)++; //会心 0x02158584
            (*position)++; //0x021ec6f8 不明
            (*position)++; //ニセ回避 0x02157f58 100%
            baseDamage = FUN_021e8458_typeD(position, 4, 34);
            (*position)++; //?? 0x02158ac4
            (*position)++; //?? 0x021e54fc
            if (!players[0].paralysis && !players[0].sleeping) {
                tmp = baseDamage * players[defender].defence;
                baseDamage = static_cast<int>(floor(tmp));
            }
            process7A8(position, baseDamage, players, defender);
            break;
        case BattleEmulator::INACTIVE_ALLY:
        case BattleEmulator::PARALYSIS:
        case BattleEmulator::INACTIVE_ENEMY:
            (*position) += 2;
            (*position)++; //関係ない
            (*position)++; //会心
            (*position)++; //回避
            FUN_0207564c(position, players[attacker].defaultATK, players[attacker].def);
            (*position)++; //不明 0x021e54fc
            baseDamage = 0;
            break;
        case CRACKLE:
            players[attacker].mp -= 8;
            (*position) += 2;
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++; //関係ない 0x021ec6f8
            (*position)++; //盾ガード 0x021586fc
            (*position)++; //偽回避 0x02157f58
            baseDamage = FUN_021e8458_typeD(position, 8, 50);
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                baseDamage = static_cast<int>(floor(tmp));
            }
            ProcessRage(position, baseDamage, players); // 適当
            if (kaisinn) {
                if (!players[1].rage) {
                    (*position)++; //会心時特殊処理　0x021e54fc
                    (*position)++; //会心時特殊処理　0x021eb8c8
                } else {
                    (*position)++; //会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }
            }
            (*position)++; //0x021e54fc
            if (!players[0].specialCharge) {
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021edaf4
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            break;
        case SPECIAL_MEDICINE:
            (*position) += 2;
            (*position)++; //0x021ec6f8 不明
            (*position)++; //0x02158584 会心
            (*position)++; //0x02157f58 ニセ回避
            baseDamage = FUN_021e8458_typeC(position, 105, 105, 15);
            (*position)++; //0x021e54fc
            if (!players[0].specialCharge) {
                (*position)++; //0x021ed7a8
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021edaf4
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].SpecialMedicineCount--;
            break;
        case MAGIC_WATER:
            players[attacker].MagicWaterCount--;
            (*position) += 2;
            (*position)++; //0x021ec6f8 不明
            (*position)++; //0x02158584 会心
            (*position)++; //0x02157f58 ニセ回避
            baseDamage = FUN_021e8458_typeC(position, 33, 33, 3);
            (*position)++; //不明 0x021e54fc
            if (!players[attacker].specialCharge) {
                (*position)++; //0x021ed7a8
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021edaf4
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            players[attacker].mp += baseDamage;
            players[attacker].mp = std::min(players[attacker].mp, players[attacker].maxMp);
            baseDamage = 0;
            break;
        case BattleEmulator::MEDICINAL_HERBS:
            players[0].SpecialMedicineCount--;
            (*position) += 2;
            (*position)++; // 関係ない
            (*position)++; // 会心判定
            (*position)++; // 回避
            baseDamage = FUN_021e8458_typeC(position, 35.0, 35.0, 5.0);
            (*position)++; // 不明
            if (!players[attacker].specialCharge) {
                (*position)++; // 必殺チャージ(敵)　0%
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021ed7a8
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            break;
        case BattleEmulator::SLEEPING:
        case BattleEmulator::CURE_SLEEPING:
            (*position) += 2;
            (*position)++; //関係ない
            (*position)++; //会心
            (*position)++; //回避
            FUN_0207564c(position, players[attacker].defaultATK, players[attacker].def);
            (*position)++; //不明
            if (!players[attacker].specialCharge && !players[attacker].sleeping && !players[attacker].paralysis) {
                (*position)++; //必殺チャージ(敵)
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021ed7a8
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            baseDamage = 0;
            break;
        case BattleEmulator::DEFENCE:
            (*position) += 2;
            (*position)++; //関係ない
            (*position)++; //会心
            (*position)++; //回避
            baseDamage = FUN_0207564c(position, players[attacker].defaultATK, players[attacker].def);
            if (baseDamage == 0) {
                baseDamage = lcg::getPercent(position, 2); //0x021e81a0
            }
            if (baseDamage != 0) {
                (*position)++; //不明 0x021e54fc
            }
            if (!players[attacker].specialCharge && !players[attacker].sleeping && !players[attacker].paralysis) {
                (*position)++; //必殺チャージ(敵) 0x021ed7a8
                if (lcg::getPercent(position, 100) < 1) {
                    //0x021edaf4
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            baseDamage = 0;
            break;
        case BattleEmulator::COUNTER:
            (*position)++; //0x02158584 会心(無効)
            if (lcg::getPercent(position, 0x2710) < 500) {
                kaisinn = true;
            }
            (*position)++; //みかわし(無効) xxx0x021587b0
        // if (!kaihi) {
            (*position)++; //盾ガード
        // }
            (*position)++; //回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (kaisinn) {
                tmp = players[attacker].atk * lcg::floatRand(position, 0.95, 1.05);
                baseDamage = static_cast<int>(floor(tmp));
            }
            ProcessRage(position, baseDamage, players);
            Player::reduceHp(players[defender], baseDamage);

            baseDamage = 0;
            break;
        case BattleEmulator::ACROBATSTAR_KAIHI:
            //(*position) += 2;
            //(*position)++;//アクロバットスター判定
            if (lcg::getPercent(position, 0x2710) < 200) {
                kaisinn = true;
            }
            (*position)++; //回避
            FUN_0207564c(position, players[attacker].atk, players[defender].def);
            if (!players[defender].specialCharge) {
                (*position)++; //0x021ed7a8
            }
            baseDamage = 0;
            break;
        case BattleEmulator::ACROBATIC_STAR:
            players[0].acrobaticStar = true;
            players[0].acrobaticStarTurn = 6;
        // if (player0_has_initiative) {
        //     players[0].specialCharge = false;
        // } else {
        //     players[0].dirtySpecialCharge = true;
        // }
            players[0].specialCharge = false;
            players[0].specialChargeTurn = 0;

            (*position) += 2;
            (*position)++; //関係ない
            (*position)++; //会心
            (*position)++; //回避
            FUN_0207564c(position, players[attacker].defaultATK, players[defender].def);
            (*position)++; //不明
            break;
        case BattleEmulator::ATTACK_ENEMY:
        case BattleEmulator::SKY_ATTACK:
        case BattleEmulator::MASSIVE_SWIPE:
            (*position) += 2;
            if (players[0].acrobaticStar) {
                percent_tmp = lcg::getPercent(position, 100);
                if (percent_tmp >= 0 && percent_tmp <= 49) {
                    return callAttackFun(ACROBATSTAR_KAIHI, position, players, attacker, defender, NowState);
                }
                if (percent_tmp >= 50 && percent_tmp < 75) {
                    return callAttackFun(COUNTER, position, players, defender, attacker, NowState);
                }
            } else {
                (*position)++;
            }


            (*position)++; //会心
            if (!players[0].paralysis && !players[0].sleeping) {
                if (!players[defender].acrobaticStar && lcg::getPercent(position, 100) < 2) {
                    kaihi = true;
                }
                if (!kaihi && lcg::getPercent(position, 100) < 1) {
                    tate = true;
                }
            }
            (*position)++; //回避

            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            if (kaihi) {
                //                if (baseDamage == 0) {
                //                    (*position)++;//0x021e81a0
                //                }

                if (!players[0].paralysis && !players[0].sleeping && !players[0].specialCharge) {
                    (*position)++; //0x021ed7a8
                }
                baseDamage = 0;
            } else if (tate) {
                //                if (baseDamage == 0) {
                //                    (*position)++;//0x021e81a0
                //                }

                if (!players[0].paralysis && !players[0].sleeping && !players[0].specialCharge) {
                    (*position)++; //0x021ed7a8
                }
                baseDamage = 0;
            } else {
                if ((Id & 0xffff) == POISON_ATTACK) {
                    if (lcg::getPercent(position, 100) < 12) {
                        //0x021e3978 毒
                        players[defender].PoisonEnable = true;
                        players[defender].PoisonTurn = 0;
                    }
                }

                tmp = static_cast<double>(baseDamage);
                if ((Id & 0xffff) == BattleEmulator::SKY_ATTACK) {
                    tmp = floor(tmp * 1.5);
                }

                //テンションがある場合、この時点でオフセットが計算されて、最低4ダメージが保証されて下の0x021e81a0でダメージがある判定になる。
                //1*(1+(30/10))で4ダメージが保証されるけど、事前に計算して定数にしとく。
                if (players[attacker].TensionLevel != 0) {
                    //TODO ダメージが正しいか調べる 特殊県産式の引数も調べる https://dragonquest9.com/?%E3%83%80%E3%83%A1%E3%83%BC%E3%82%B8%E3%81%AB%E3%81%A4%E3%81%84%E3%81%A6#tension
                    tmp *= Enemy_TensionTable[players[attacker].TensionLevel - 1];
                    tmp += (players[attacker].TensionLevel * 4); //4 = 1*(1+(30/10))
                    players[attacker].TensionLevel = 0;
                }

                baseDamage = static_cast<int>(floor(tmp));


                //防御が適応される時期を調べる
                if (baseDamage == 0) {
                    // && players[0].defence != 0.1
                    baseDamage = lcg::getPercent(position, 2); //TODO: 0x021e81a0
                    if (baseDamage == 1) {
                        defenseFlag = true;
                    }
                }

                if (!defenseFlag && !players[0].paralysis && !players[0].sleeping) {
                    tmp = baseDamage * players[defender].defence;
                    baseDamage = static_cast<int>(floor(tmp));
                }


                if (baseDamage != 0) {
                    (*position)++; //目を覚ました
                    (*position)++; //不明
                }


                if (baseDamage != 0 && players[0].sleeping) {
                    players[0].sleeping = false;
                    players[0].sleepingTurn = -1;
                }

                process7A8(position, baseDamage, players, defender);
            }

            players[attacker].TensionLevel = 0;

            break;
        case BattleEmulator::HEAL:
            players[attacker].mp -= 2;
            (*position) += 2;
            (*position)++; //関係ない
            if (lcg::getPercent(position, 0x2710) < 100) {
                kaisinn = true;
            }
            (*position)++; //回避
            baseDamage = FUN_021e8458_typeD(position, 5, 35);
            if (kaisinn) {
                tmp = baseDamage * lcg::floatRand(position, 1.5, 2.0);
                baseDamage = static_cast<int>(floor(tmp));
            }
            (*position)++; //不明
            if (!players[attacker].specialCharge) {
                (*position)++; //関係ない
            }
        //0x021eb8c8, randIntRange: 0x021eb8f0 怒り狂っている場合←の消費が発生しない。
            if (!players[1].rage) {
                (*position)++;
            }
            (*position)++; //?
            if (kaisinn) {
                if (!players[1].rage) {
                    (*position)++; //会心時特殊処理　0x021e54fc
                    (*position)++; //会心時特殊処理　0x021eb8c8
                } else {
                    (*position)++; //会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }
            }
            if (!players[0].paralysis && !players[0].sleeping) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
            break;
        case BattleEmulator::ATTACK_ALLY:
        case BattleEmulator::MERCURIAL_THRUST:
        case BattleEmulator::MIRACLE_SLASH:
            (*position) += 2;
            (*position)++;
        //会心
            percent_tmp = lcg::getPercent(position, 0x2710);
            if (
                ((Id & 0xffff) == BattleEmulator::ATTACK_ALLY && percent_tmp < 500) ||
                ((Id & 0xffff) == BattleEmulator::MERCURIAL_THRUST && percent_tmp < 250) ||
                ((Id & 0xffff) == BattleEmulator::MERCURIAL_THRUST && percent_tmp < 125)
            ) {
                kaisinn = true;
            }

        //みかわし(相手)
            if (!players[0].paralysis) {
                // if (lcg::getPercent(position, 100) < -1) {
                //     kaihi = true;
                // }
                (*position)++; //0x021587b0 みかわし
                //if (!kaihi) {
                (*position)++; //盾ガード(幼女は盾を持っていないので0%)
                //}
            }

            (*position)++; //回避
            baseDamage = FUN_0207564c(position, players[attacker].atk, players[defender].def);

            tmp = static_cast<double>(baseDamage);

            if (kaisinn) {
                //0x020759ec
                if ((Id & 0xffff) == BattleEmulator::MERCURIAL_THRUST || (Id & 0xffff) ==
                    BattleEmulator::MIRACLE_SLASH) {
                    tmp *= lcg::floatRand(position, 1.5, 2.0);
                } else {
                    tmp = OffensivePower * lcg::floatRand(position, 0.95, 1.05);
                }
            }

        //ミラクルのやつ
            if ((Id % 0xffff) == BattleEmulator::MIRACLE_SLASH) {
                players[attacker].mp -= 4;
                tmp *= 1.25;
            }

            baseDamage = static_cast<int>(floor(tmp));

            ProcessRage(position, baseDamage, players);
            (*position)++; //目を覚ました
            (*position)++; //不明
            if (kaisinn) {
                if (!players[1].rage) {
                    (*position)++; //会心時特殊処理　0x021e54fc
                    (*position)++; //会心時特殊処理　0x021eb8c8
                } else {
                    (*position)++; //会心時特殊処理　既に怒り狂ってる場合は1消費になる
                }
            }
            if (players[defender].hp - baseDamage >= 0) {
                if (!players[attacker].specialCharge && lcg::getPercent(position, 100) < 1) {
                    players[attacker].specialCharge = true;
                    players[attacker].specialChargeTurn = 6;
                }
            }
        //ミラクルのやつ
            if ((Id % 0xffff) == BattleEmulator::MIRACLE_SLASH && baseDamage != 0) {
                tmp = static_cast<double>(baseDamage);
                tmp *= 0.25;
                Player::heal(players[attacker], static_cast<int>(floor(tmp)));
            }
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
    if (!players[defender].paralysis && !players[defender].sleeping) {
        if (players[defender].hp > baseDamage) {
            //hpが0以下の場合必殺チャージの判定は発生しない。
            //必殺チャージ(敵)
            if (!players[defender].specialCharge) {
                if (baseDamage == 0) {
                    (*position)++;
                    return;
                }
                auto percent_tmp = lcg::getPercent(position, 100);
                double tmp = baseDamage;
                if (!players[0].paralysis && !players[0].sleeping) {
                    tmp *= players[defender].defence;
                }
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
    } /*else if (players[defender].sleeping) {
        (*position)++;
    }*/
}

int BattleEmulator::ProcessEnemyRandomAction2A(int *position) {
    //0x0208aca8
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


int BattleEmulator::ProcessEnemyRandomAction44(int *position) {
    //0x0208aca8
    int rnd = lcg::getPercent(position, 0x100) + 1;
    if (rnd <= 68) return ATTACK_ENEMY;
    if (rnd <= 68 + 58) return ATTACK_ENEMY;
    if (rnd <= 68 + 58 + 48) return TIDAL_WAVE;
    if (rnd <= 68 + 58 + 48 + 38) return ATTACK_ENEMY;
    if (rnd <= 68 + 58 + 48 + 38 + 27) return MASSIVE_SWIPE;
    return ATTACK_ENEMY;
}

int BattleEmulator::CalculateMoreHealBase(Player *players) {
    //ベホイミ
    double tmp1 = (players[0].HealPower - 200) * 0.5194;
    auto tmp2 = static_cast<int>(floor(tmp1));
    return 185 + tmp2;
}

int BattleEmulator::CalculateMidHealBase(Player *players) {
    //ｂ
    double tmp1 = (players[0].HealPower - 100) * 0.2392;
    auto tmp2 = static_cast<int>(floor(tmp1));
    return 85 + tmp2;
}

void BattleEmulator::RecalculateBuff(Player *players) {
    // 定数の倍率を格納した配列
    //const double ATKMultipliers[] = {0.5, 0.75, 1.0, 1.25, 1.5};
    const double DEFMultipliers[] = {0.25, 0.5, 1.0, 1.5, 2.0};
    const double SpeedMultipliers[] = {0.25, 0.5, 1.0, 1.5, 2.0};

    // BuffLevel に +2 して配列インデックスに変換
    int index = players[0].BuffLevel + 2;
    int index1 = players[0].speedLevel + 2;

    // インデックスが範囲外でないかチェック（-2 <= BuffLevel <= 2 の範囲であることを確認）
    if (index >= 0 && index < 5) {
        players[0].def = static_cast<int>(floor(players[0].defaultDEF * DEFMultipliers[index]));
    }

    if (index1 >= 0 && index1 < 5) {
        players[0].speed = static_cast<int>(floor(players[0].defaultSpeed * SpeedMultipliers[index1]));
    }

    //
    // int index1 = players[0].AtkBuffLevel + 2;
    // if (index >= 0 && index < 5) {
    //     players[0].atk = static_cast<int>(floor(players[0].defaultATK * ATKMultipliers[index1]));
    // }
}

void BattleEmulator::ProcessRage(int *position, int baseDamage, Player *players) {
    auto percent1 = FUN_021dbc04(players[1].hp - baseDamage, players[1].maxHp);
    if (percent1 < 0.5) {
        double percent = FUN_021dbc04(players[1].hp, players[1].maxHp);
        if (percent >= 0.5) {
            if (!players[0].rage) {
                (*position)++;
                players[0].rage = true;
                players[0].rageTurns = lcg::intRangeRand(position, 2, 4);
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

int BattleEmulator::ProcessMagicBurst(int *position) {
    auto rand1 = lcg::floatRand(position, 0.9, 1.0);
    auto rand2 = lcg::floatRand(position, 0.9, 1.1);
    auto damage = 30 * 6 * rand1;
    auto guaranteed = 160 * rand2;
    if (damage > guaranteed) {
        return static_cast<int>(floor(damage));
    } else {
        return static_cast<int>(floor(guaranteed));
    }
}
