//
// Created by Owner on 2024/11/21.
//

#include "GeneticAlgorithm.h"
#include <vector>
#include <random>
#include <functional>
#include <queue>
#include "BattleEmulator.h"
#include "Genome.h"
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


void updateCompromiseScore(Genome &genome) {
    //敵の行動に応じた減点
    for (int i = 0; i < 2; ++i) {
        if (genome.EActions[i] == BattleEmulator::LULLAB_EYE) {
            genome.compromiseScore -= 2; // あやしいひとみ
        } else if (genome.EActions[i] == BattleEmulator::DISRUPTIVE_WAVE) {
            genome.compromiseScore -= 10;
        }
    }

    // 味方の状態に応じた減点
    if (genome.Aactions == BattleEmulator::SLEEPING || genome.Aactions == BattleEmulator::CURE_SLEEPING || genome.
        Aactions == BattleEmulator::TURN_SKIPPED) {
        genome.compromiseScore -= 1; // 睡眠
    } else if (genome.Aactions == BattleEmulator::PARALYSIS || genome.Aactions == BattleEmulator::CURE_PARALYSIS) {
        genome.compromiseScore -= 100; // 麻痺（許容不可）
    } else {
        genome.compromiseScore = std::min(0, genome.compromiseScore + 1);
    }
}

// 遺伝的アルゴリズム実行
Genome GeneticAlgorithm::RunGeneticAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations,
                                             int actions[350]) {
    std::mt19937 rng(seed);
    auto *position = new int(1);
    auto *nowState = new uint64_t(0);
    auto counter = 0;
    Genome genome;
    ActionBanManager Bans;
    for (int i = 0; i < 350; ++i) {
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
    //std::priority_queue<Genome, std::vector<Genome>, std::greater<> > que;
    std::priority_queue<Genome> que;

    genome = {};

    genome.EnemyPlayer = players[1];
    genome.AllyPlayer = players[0];
    genome.EActions[0] = -1;
    genome.EActions[1] = -1;
    genome.Aactions = -1;
    genome.fitness = 0;
    genome.turn = turns + 1;
    genome.Initialized = false;
    genome.compromiseScore = 0;
    genome.isEliminated = false;
    genome.processed = 0;
    genome.Visited = 0;
    genome.position = 1;
    genome.state = BattleEmulator::TYPE_2A;
    genome.canMove = false;

    for (int i = 0; i < 350; ++i) {
        if (actions[i] == -1 || actions[i] == 0) {
            genome.actions[i] = -1;
            break;
        } else {
            genome.actions[i] = actions[i];
        }
    }

    que.push(genome);

    Genome currentGenome;
    while (!que.empty() && (maxGenerations == -1 || maxGenerations > counter)) {
        currentGenome = que.top();
        que.pop();
        // if (currentGenome.isEliminated) {
        //     continue;
        // }

        turns = currentGenome.turn;

        if (currentGenome.Initialized && turns > 1 && currentGenome.actions[turns - 1] != 0) {
            Bans.ban_action(turns - 1, currentGenome.actions[turns - 1]);
        }

        CopedPlayers[0] = currentGenome.AllyPlayer;
        CopedPlayers[1] = currentGenome.EnemyPlayer;
        (*position) = currentGenome.position;
        (*nowState) = currentGenome.state;

        auto tmpgenomu = currentGenome;

        std::optional<BattleResult> result;
        result = BattleResult();
        BattleEmulator::Main(position, currentGenome.turn - currentGenome.processed, currentGenome.actions,
                             CopedPlayers,
                             result, seed,
                             nullptr, nullptr, -1, nowState);

        if (CopedPlayers[0].hp <= 0) {
            continue;
        }

        counter1 = 0;
        for (int i = result->position - 3; i < result->position; ++i) {
            if (result->isEnemy[i]) {
                Eactions[counter1] = result->actions[i];
                Edamage[counter1] = result->damages[i];
                currentGenome.EActions[counter1++] = result->actions[i];
            } else {
                Aactions = result->actions[i];
                currentGenome.Aactions = result->actions[i];
            }
        }

        if (currentGenome.Visited == 0) {
            updateCompromiseScore(currentGenome);
        }
        //この時点で副作用が分かる。

        auto backToPast = false;

        if (Eactions[1] == BattleEmulator::LULLAB_EYE) {
            backToPast = true;
        }
        if (Eactions[0] == BattleEmulator::CRITICAL_ATTACK || Eactions[1] == BattleEmulator::CRITICAL_ATTACK) {
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

        if (Eactions[0] == BattleEmulator::DISRUPTIVE_WAVE || Eactions[1] == BattleEmulator::DISRUPTIVE_WAVE) {
            backToPast = true;
        }

        if (currentGenome.Initialized && backToPast && currentGenome.Visited == 0) {
            if (!backToPasted) {
                backToPasted = true;
            }
            currentGenome.fitness -= 2;
            currentGenome.Visited++;
            que.push(currentGenome);
            counter++;
            continue;
        }
        backToPasted = false;
        currentGenome.Visited = 0;

        bool skip = false;

        // if (genome.actions[turns - 1] == BattleEmulator::DEFENDING_CHAMPION) {
        //     skip = true;
        // }

        if (currentGenome.canMove || Eactions[0] == BattleEmulator::MAGIC_BURST || Eactions[1] ==
            BattleEmulator::MAGIC_BURST) {
            skip = false;
            currentGenome.canMove = false;
        } else {
            if (Aactions == BattleEmulator::SLEEPING || Aactions == BattleEmulator::PARALYSIS || Aactions ==
                BattleEmulator::TURN_SKIPPED) {
                currentGenome.canMove = false;
                skip = true;
            } else if (Aactions == BattleEmulator::CURE_SLEEPING || Aactions == BattleEmulator::CURE_PARALYSIS) {
                currentGenome.canMove = true;
                skip = true;
            } else {
                currentGenome.canMove = true;
            }
        }


        currentGenome.canMove = !CopedPlayers[0].sleeping;

        auto baseFitness = currentGenome.fitness;

        if (skip) {
            if (!currentGenome.Initialized) {
                //todo
                currentGenome.actions[turns - 1] = BattleEmulator::ATTACK_ALLY;
                currentGenome.fitness += 10;
                currentGenome.position = (*position);
                currentGenome.state = (*nowState);
                currentGenome.turn = turns + 1;
                currentGenome.processed = turns;
                currentGenome.AllyPlayer = CopedPlayers[0];
                currentGenome.EnemyPlayer = CopedPlayers[1];
                que.push(currentGenome);
                counter++;
                continue;
            }
            if (currentGenome.Visited == 0) {
                currentGenome.fitness -= 2;
                currentGenome.Visited++;
                que.push(currentGenome);
                counter++;
                continue;
            }
        }
        if (Eactions[0] == BattleEmulator::MAGIC_BURST || Eactions[1] == BattleEmulator::MAGIC_BURST) {
            currentGenome.actions[turns - 1] = BattleEmulator::DEFENDING_CHAMPION;
            currentGenome.fitness = baseFitness + 10;
            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];
            currentGenome.Initialized = false;

            que.push(currentGenome);
            continue;
        }

        auto AllyPlayer = CopedPlayers[0];
        auto EnemyPlayer = CopedPlayers[1];

        currentGenome.Initialized = true;
        //if (!skip) {
        if (AllyPlayer.AtkBuffLevel == 0 && AllyPlayer.BuffLevel == 2 && !Bans.is_action_banned(
                BattleEmulator::DOUBLE_UP, turns)) {
            action = BattleEmulator::DOUBLE_UP;
            currentGenome.fitness = baseFitness + 8 + static_cast<int>(rng() % 6);
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }
        if (AllyPlayer.BuffLevel <= 1 && !Bans.is_action_banned(BattleEmulator::BUFF, turns)) {
            action = BattleEmulator::BUFF;
            currentGenome.fitness = baseFitness + 8 + static_cast<int>(rng() % 6);
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }
        if (AllyPlayer.MagicMirrorTurn < 3 && !Bans.is_action_banned(BattleEmulator::MAGIC_MIRROR, turns)) {
            action = BattleEmulator::MAGIC_MIRROR;
            currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 6);
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }
        if (!Bans.is_action_banned(BattleEmulator::MULTITHRUST, turns)) {
            action = BattleEmulator::MULTITHRUST;
            if (AllyPlayer.AtkBuffLevel != 0&&AllyPlayer.hasMagicMirror) {
                currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 6);
            }else {
                currentGenome.fitness = baseFitness + 6 + static_cast<int>(rng() % 6);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }
        if (!Bans.is_action_banned(BattleEmulator::MORE_HEAL, turns) && (AllyPlayer.hp / AllyPlayer.maxHp) <
            0.7) {
            action = BattleEmulator::MORE_HEAL;
            currentGenome.fitness = baseFitness + 5 + static_cast<int>(rng() % 6);
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }

        if (!Bans.is_action_banned(BattleEmulator::FULLHEAL, turns) && (AllyPlayer.hp / AllyPlayer.maxHp) <
            0.7) {
            action = BattleEmulator::FULLHEAL;
            currentGenome.fitness = baseFitness + 5 + static_cast<int>(rng() % 6);
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState);
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }
        counter++;
    }

    delete position;
    delete nowState;
    if (que.empty()) {
        return currentGenome;
    }
    return que.top();
}
