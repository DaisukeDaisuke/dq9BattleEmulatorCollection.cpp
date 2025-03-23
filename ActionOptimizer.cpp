//
// Created by Owner on 2024/11/21.
//

#include "ActionOptimizer.h"
#include <vector>
#include <random>
#include <functional>
#ifdef defined(MULTITHREADING)
#include <future>
#endif
#include <memory>
#include <queue>
#include "BattleEmulator.h"
#include "Genome.h"
#include "ActionBanManager.h"
#include "HeapQueue.h"
#include "lcg.h"

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
        if (genome.EActions[i] == BattleEmulator::KASAP) {
            genome.compromiseScore -= 2;
        }
    }

    // 味方の状態に応じた減点
    if (genome.Aactions == BattleEmulator::SLEEPING || genome.Aactions == BattleEmulator::CURE_SLEEPING || genome.
        Aactions == BattleEmulator::TURN_SKIPPED) {
        genome.compromiseScore -= 1; // 睡眠
    } else {
        genome.compromiseScore = std::min(0, genome.compromiseScore + 1);
    }
}

#ifdef defined(MULTITHREADING)

// 並列処理のための関数
Genome ActionOptimizer::RunAlgorithmSingleThread(const Player players[2], uint64_t seed, int turns, int maxGenerations, int actions[], int start, int end) {
    auto seed1 = seed;
    auto turns1 = turns;
    auto maxGenerations1 = maxGenerations;

    Genome bestGenome = {};
    bestGenome.turn = INT_MAX;

    std::unique_ptr<int> position = std::make_unique<int>(1);
    std::unique_ptr<uint64_t> nowState = std::make_unique<uint64_t>(0);

    for (int i = start; i < end; ++i) {
        Player localPlayers[2] = { players[0], players[1] };
        Genome candidate = RunAlgorithm(localPlayers, seed1, turns1, maxGenerations1, actions, i * 2);

        std::optional<BattleResult> result1;
        result1 = BattleResult();

        Player localPlayers1[2] = { players[0], players[1] };

        *position = 1;
        *nowState = 0;

        result1->clear();
        BattleEmulator::Main(position.get(), 100, candidate.actions, localPlayers1, result1, seed, nullptr, nullptr, -1, nowState.get());

        if (localPlayers1[0].hp >= 0 && localPlayers1[1].hp == 0) {
            candidate.turn = result1->turn;
            if (candidate.turn < bestGenome.turn) {
                candidate.AllyPlayer = localPlayers1[0];
                candidate.EnemyPlayer = localPlayers1[1];
                bestGenome = candidate;
            }
        }
    }

    return bestGenome;
}




// メインの並列処理関数
Genome ActionOptimizer::RunAlgorithmAsync(const Player players[2], uint64_t seed, int turns, int totalIterations, int actions[350]) {
    lcg::init(seed, true);
    int numThreads = 6;
    int chunkSize = totalIterations / numThreads;

    std::vector<std::future<Genome>> futures;
    futures.reserve(numThreads); // ✅ メモリ確保でスコープ外の問題を防ぐ

    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? totalIterations : start + chunkSize;

        futures.push_back(std::async(std::launch::async, RunAlgorithmSingleThread,
                                     std::cref(players), seed, turns, 5000, actions, start, end));
    }

    Genome bestGenome = {};
    bestGenome.turn = INT_MAX;

    for (auto& future : futures) {
        Genome candidate = future.get();
        if (candidate.turn < bestGenome.turn) {
            bestGenome = candidate;
        }
    }

    return bestGenome;
}

#endif

// オレオレアルゴリズム実行
Genome ActionOptimizer::RunAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations,
                                     int actions[350], int seedOffset) {
    std::mt19937 rng(seed + seedOffset);
    bool CrackleEnable = (rng() % 2) == 0;
    std::unique_ptr<int> position = std::make_unique<int>(1);
    std::unique_ptr<uint64_t> nowState = std::make_unique<uint64_t>(0);
    auto counter = 0;
    Genome genome;
    ActionBanManager Bans;
    for (int i = 0; i < 350; ++i) {
        if (actions[i] == 0 || actions[i] == -1) {
            break;
        }
        genome.actions[i] = actions[i];
    }
    Player CopedPlayers[2];
    auto action = -1;
    auto Aactions = -1;
    // auto Adamage = -1;
    // int Eactions[2] = {-1, -1};
    // int Edamage[2] = {-1, -1};
    auto counter1 = 0;
    //std::priority_queue<Genome, std::vector<Genome>, std::greater<> > que;
    //std::priority_queue<Genome> que;
    HeapQueue que(300);

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
            genome.actions[i] = BattleEmulator::ATTACK_ALLY;
            break;
        } else {
            genome.actions[i] = actions[i];
        }
    }

    que.push(genome);
    std::optional<BattleResult> result;
    result = BattleResult();

    Genome BaseGenome = {};
    BaseGenome.turn = INT32_MAX - 1;
    bool found = false;
    Genome currentGenome;
    while (!que.empty() && (maxGenerations == -1 || maxGenerations > counter)) {
        currentGenome = que.top();
        que.pop();

        turns = currentGenome.turn;

        if (turns > 40) {
            continue;
        }

        if (currentGenome.Initialized && turns > 1 && currentGenome.actions[turns - 1] > 0) {
            Bans.ban_action(turns - 1, currentGenome.actions[turns - 1]);
        }

        CopedPlayers[0] = currentGenome.AllyPlayer;
        CopedPlayers[1] = currentGenome.EnemyPlayer;
        *position = currentGenome.position;
        *nowState = currentGenome.state;

        const auto tmpgenomu = currentGenome;
        result->clear(); //メモリ新規確保よりこっちのほうが早い

        BattleEmulator::Main(position.get(), currentGenome.turn - currentGenome.processed, currentGenome.actions,
                             CopedPlayers,
                             result, seed,
                             nullptr, nullptr, -1, nowState.get());

        if (CopedPlayers[0].hp <= 0) {
            continue;
        }

        if (CopedPlayers[1].hp <= 0) {
            currentGenome.fitness += 100;
            //que.push(currentGenome);
            if (BaseGenome.turn > currentGenome.turn) {
                BaseGenome = currentGenome; //最適解を更新
                found = true;
            }
            continue;
        }

        if (BaseGenome.turn < currentGenome.turn) {
            continue; //最適解より長い回答は求めてない
        }

        counter1 = 0;
        for (int i = result->position - 3; i < result->position; ++i) {
            if (result->isEnemy[i]) {
                //Eactions[counter1] = result->actions[i];
                //Edamage[counter1] = result->damages[i];
                currentGenome.EActions[counter1++] = result->actions[i];
            } else {
                Aactions = result->actions[i];
                //Adamage = result->damages[i];
                currentGenome.Aactions = result->actions[i];
            }
        }

        if (currentGenome.Visited == 0) {
            updateCompromiseScore(currentGenome);
        }
        //この時点で副作用が分かる。

        auto backToPast = false;

        if (tmpgenomu.Initialized && currentGenome.AllyPlayer.BuffLevel != 0) {
            backToPast = true;
        }

        // if (currentGenome.Visited == 1) {
        //     std::cout << std::endl;
        // }

        if (currentGenome.Initialized && backToPast && currentGenome.Visited == 0) {
            // &&
            currentGenome.fitness -= 5;
            currentGenome.Visited++;
            que.push(currentGenome);
            counter++;
            continue;
        }
        bool skip = false;

        if (Aactions == BattleEmulator::SLEEPING || Aactions == BattleEmulator::PARALYSIS || Aactions ==
            BattleEmulator::TURN_SKIPPED) {
            //canmove1 = false;
            skip = true;
        }

        auto baseFitness = tmpgenomu.fitness;

        if (skip) {
            if (!currentGenome.Initialized) {
                //todo
                currentGenome.actions[turns - 1] = BattleEmulator::ATTACK_ALLY;
                currentGenome.fitness += 10;
                currentGenome.position = *(position);
                currentGenome.state = *(nowState);
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
        auto AllyPlayer = CopedPlayers[0];
        //auto EnemyPlayer = CopedPlayers[1];


        auto AllyPlayerPre = tmpgenomu.AllyPlayer;
        //auto EnemyPlayerPre = tmpgenomu.EnemyPlayer;

        currentGenome.Initialized = true;
        //if (!skip) {


        if (AllyPlayer.PoisonEnable == true && !Bans.is_action_banned(BattleEmulator::SPECIAL_ANTIDOTE, turns) &&
            AllyPlayerPre.SpecialAntidoteCount > 0) {
            action = BattleEmulator::SPECIAL_ANTIDOTE;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 4 + static_cast<int>(rng() % 6);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        } else if (!Bans.is_action_banned(BattleEmulator::SPECIAL_MEDICINE, turns) && AllyPlayerPre.SpecialMedicineCount
                   > 0) {
            action = BattleEmulator::SPECIAL_MEDICINE;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 4 + static_cast<int>(rng() % 6);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }

        if (!Bans.is_action_banned(BattleEmulator::ATTACK_ALLY, turns)) {
            action = BattleEmulator::ATTACK_ALLY;
            auto visited = tmpgenomu.Visited;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 15 + static_cast<int>(rng() % 6);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            auto ehp1 = tmpgenomu.EnemyPlayer.hp;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            if (visited == 0 && ehp1 - currentGenome.EnemyPlayer.hp > 70) {
                currentGenome.fitness += 30;
            }

            que.push(currentGenome);
        }

        if (!Bans.is_action_banned(BattleEmulator::DEFENCE, turns)) {
            action = BattleEmulator::DEFENCE;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 6 + static_cast<int>(rng() % 6);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }

        if (AllyPlayerPre.mp >= 4) {
            if (!Bans.is_action_banned(BattleEmulator::MIRACLE_SLASH, turns)) {
                action = BattleEmulator::MIRACLE_SLASH;
                if (tmpgenomu.Visited >= 1) {
                    currentGenome.fitness = baseFitness; // 固定値に
                    currentGenome.Visited = 0;
                } else {
                    currentGenome.fitness = baseFitness + 14 + static_cast<int>(rng() % 6); // 通常は乱数を利用
                }
                currentGenome.actions[turns - 1] = action;

                CopedPlayers[0] = tmpgenomu.AllyPlayer;
                CopedPlayers[1] = tmpgenomu.EnemyPlayer;

                (*position) = tmpgenomu.position;
                (*nowState) = tmpgenomu.state;

                BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                     CopedPlayers,
                                     (std::optional<BattleResult> &) std::nullopt, seed,
                                     nullptr, nullptr, -2, nowState.get());
                currentGenome.position = (*position);
                currentGenome.state = (*nowState);
                currentGenome.turn = turns + 1;
                currentGenome.processed = turns;
                currentGenome.AllyPlayer = CopedPlayers[0];
                currentGenome.EnemyPlayer = CopedPlayers[1];


                que.push(currentGenome);
            }
        }

        if (!Bans.is_action_banned(BattleEmulator::FLEE_ALLY, turns)) {
            action = BattleEmulator::FLEE_ALLY;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 5 + static_cast<int>(rng() % 13);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }

        if (AllyPlayerPre.specialCharge == true && !Bans.is_action_banned(BattleEmulator::ACROBATIC_STAR, turns)) {
            action = BattleEmulator::ACROBATIC_STAR;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 13);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
            currentGenome.position = (*position);
            currentGenome.state = (*nowState);
            currentGenome.turn = turns + 1;
            currentGenome.processed = turns;
            currentGenome.AllyPlayer = CopedPlayers[0];
            currentGenome.EnemyPlayer = CopedPlayers[1];

            que.push(currentGenome);
        }

        if (CrackleEnable && AllyPlayerPre.mp >= 8 && !Bans.is_action_banned(BattleEmulator::CRACKLE, turns)) {
            action = BattleEmulator::CRACKLE;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 7 + static_cast<int>(rng() % 20);
            }
            currentGenome.actions[turns - 1] = action;

            CopedPlayers[0] = tmpgenomu.AllyPlayer;
            CopedPlayers[1] = tmpgenomu.EnemyPlayer;

            (*position) = tmpgenomu.position;
            (*nowState) = tmpgenomu.state;

            BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                                 CopedPlayers,
                                 (std::optional<BattleResult> &) std::nullopt, seed,
                                 nullptr, nullptr, -2, nowState.get());
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

    if (found) {
        return BaseGenome;
    }
    Genome bestGenome = que.empty() ? currentGenome : static_cast<Genome>(que.top());
    return bestGenome;
}
