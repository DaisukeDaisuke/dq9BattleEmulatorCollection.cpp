//
// Created by Owner on 2024/11/21.
//

#include "GeneticAlgorithm.h"
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include "BattleEmulator.h"
#include "Genome.h"
#include "lcg.h"
#include "GenomeLogger.h"
#include "ActionBanManager.h"

constexpr int NUM_GENERATIONS = 100;
constexpr int POPULATION_SIZE = 50;
constexpr int GENE_LENGTH = 10; // 行動配列の長さ

constexpr int32_t actions1[13] = {
        BattleEmulator::BUFF,
        BattleEmulator::MAGIC_MIRROR,
        BattleEmulator::MORE_HEAL,
        BattleEmulator::DOUBLE_UP,
        BattleEmulator::MULTITHRUST,
        BattleEmulator::MIDHEAL,
        BattleEmulator::FULLHEAL,
        BattleEmulator::DEFENDING_CHAMPION,
        BattleEmulator::SAGE_ELIXIR,
        BattleEmulator::ELFIN_ELIXIR,
        BattleEmulator::SPECIAL_MEDICINE,
        BattleEmulator::ATTACK_ALLY,
        BattleEmulator::DEFENCE,
};

constexpr int ACTION_COUNT = sizeof(actions1) / sizeof(actions1[0]);

// 適応度関数
BattleResult
EvaluateFitness(const Genome &genome, int *position, Player *players, uint64_t seed, uint64_t *nowState, int turns) {
    // バトルエミュを実行してスコアを計算
    (*position) = 1;
    (*nowState) = BattleEmulator::TYPE_2A;
    std::optional<BattleResult> result;
    result = BattleResult();
    BattleEmulator::Main(position, turns, genome.actions, players, result, seed,
                         nullptr, nullptr, -1, nowState);

    return result.value();
    //return  + ((static_cast<int>(((*nowState) >> 12) & 0xfffff) + 1) * 0.125); // ターンが短いほどスコアが高い
}

// 遺伝的アルゴリズム実行
Genome GeneticAlgorithm::RunGeneticAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations,
                                             int actions[500]) {
    std::mt19937 rng(seed);
    auto *position = new int(1);
    auto *nowState = new uint64_t(0);
    int PreviousPosition = 1;
    uint64_t PreviousState = BattleEmulator::TYPE_2A;
    auto *PreviousActions = actions;
    auto counter = 0;
    auto CanAct = false;
    Genome genome;
    GenomeLogger Logger;
    ActionBanManager Bans;
    for (int i = 0; i < 500; ++i) {
        if (actions[i] == 0 || actions[i] == -1) {
            break;
        }
        genome.actions[i] = actions[i];
    }

    Player nowPlayer[2];
    nowPlayer[0] = players[0];
    nowPlayer[1] = players[1];
    Player CopedPlayers[2];
    auto action = -1;
    auto Aactions = -1;
    int Eactions[2] = {-1, -1};
    int Edamage[2] = {-1, -1};
    auto counter1 = 0;
    auto processed = 0;
    auto Initialized = false;
    auto backToPasted = false;
    auto Past = turns;
    while (maxGenerations >= counter) {
        if (Eactions[0] == BattleEmulator::MEDITATION || Eactions[1] == BattleEmulator::MEDITATION) {
            if (!Initialized) {
                genome.actions[turns] = BattleEmulator::DEFENDING_CHAMPION;
            }/*else if(Past < turns-1){
                auto tmp = Logger.getLog(1);
                nowPlayer[0] = tmp.AllyPlayer;
                nowPlayer[0] = tmp.EnemyPlayer;
                PreviousPosition = tmp.position;
                PreviousState = tmp.state;
                Eactions[0] = -1;
                Eactions[1] = -1;
                continue;
            }*/
        }


        genome.EnemyPlayer = nowPlayer[1];
        genome.AllyPlayer = nowPlayer[0];
        genome.position = (*position);
        genome.state = (*nowState);
        genome.EActions[0] = Eactions[0];
        genome.EActions[1] = Eactions[1];
        genome.Aactions = Aactions;

        Logger.saveLog(genome);

        CopedPlayers[0] = nowPlayer[0];
        CopedPlayers[1] = nowPlayer[1];
        (*position) = PreviousPosition;
        (*nowState) = PreviousState;
        std::optional<BattleResult> result;
        result = BattleResult();
        BattleEmulator::Main(position, turns + 1 - processed, genome.actions, CopedPlayers, result, seed,
                             nullptr, nullptr, -1, nowState);


        counter1 = 0;
        for (int i = result->position - 3; i < result->position; ++i) {
            if (result->isEnemy[i]) {
                Eactions[counter1] = result->actions[i];
                Edamage[counter1++] = result->damages[i];
            } else {
                Aactions = result->actions[i];
            }
        }

        //この時点で副作用が分かる。

        auto backToPast = false;

        if (Eactions[1] == BattleEmulator::LULLAB_EYE) {
            backToPast = true;
        }
        if (Eactions[0] == BattleEmulator::LULLAB_EYE) {
            if ((Eactions[1] != BattleEmulator::ATTACK_ENEMY && Eactions[1] != BattleEmulator::LIGHTNING_STORM &&
                 Eactions[1] != BattleEmulator::CRITICAL_ATTACK &&
                 Eactions[1] != BattleEmulator::ULTRA_HIGH_SPEED_COMBO)) {
                backToPast = true;
            } else if (Edamage[1] == 0) {
                backToPast = true;
            }
        }

        if (Initialized && backToPast) {
            if (!backToPasted) {
                Bans.previous_turn();
                backToPasted = true;
            }
            auto tmp = Logger.getLog(2);
            CopedPlayers[0] = tmp.AllyPlayer;
            CopedPlayers[1] = tmp.EnemyPlayer;
            PreviousPosition = tmp.position;
            PreviousState = tmp.state;
            Eactions[0] = tmp.EActions[0];
            Eactions[1] = tmp.EActions[1];
            Aactions = tmp.Aactions;
            action = -1;
            turns -= 2;
        }else{
            Bans.advance_turn();
            backToPasted = false;
        }


        if (genome.actions[turns] == BattleEmulator::DEFENDING_CHAMPION) {
            goto update;
        }

        if (Aactions == BattleEmulator::SLEEPING || Aactions == BattleEmulator::PARALYSIS) {
            action = BattleEmulator::ATTACK_ALLY;
            goto update;
        } else if (Aactions == BattleEmulator::CURE_SLEEPING || Aactions == BattleEmulator::CURE_PARALYSIS) {
            action = BattleEmulator::ATTACK_ALLY;
            goto update;
        }

        if (CopedPlayers[0].AtkBuffLevel == 0 && CopedPlayers[0].BuffLevel == 2 &&
            !Bans.is_action_banned(BattleEmulator::DOUBLE_UP)) {
            action = BattleEmulator::DOUBLE_UP;
            Bans.ban_current_turn_action(action);
        } else if (CopedPlayers[0].BuffLevel <= 1 && !Bans.is_action_banned(BattleEmulator::BUFF)) {
            action = BattleEmulator::BUFF;
            Bans.ban_current_turn_action(action);
        } else if (CopedPlayers[0].MagicMirrorTurn < 3 && !Bans.is_action_banned(BattleEmulator::MAGIC_MIRROR)) {
            action = BattleEmulator::MAGIC_MIRROR;
            Bans.ban_current_turn_action(action);
        } else if (!Bans.is_action_banned(BattleEmulator::MULTITHRUST)) {
            action = BattleEmulator::MULTITHRUST;
            Bans.ban_current_turn_action(action);
        }

        Initialized = true;

        update:
        PreviousPosition = (*position);
        PreviousState = (*nowState);

        turns++;
        processed = turns;
        genome.actions[turns] = action;
        nowPlayer[0] = CopedPlayers[0];
        nowPlayer[1] = CopedPlayers[1];

        counter++;
    }

    delete position;
    delete nowState;
    return genome;
}