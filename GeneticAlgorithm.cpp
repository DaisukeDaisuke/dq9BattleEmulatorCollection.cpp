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

double calculateFitness(double hp, double maxHp, double k) {
    double x = (hp / maxHp) * 0.1;
    return std::exp(k * x) - 1.0; // スコア計算
}

// 適応度関数
double
EvaluateFitness(const Genome &genome, int *position, Player *players, uint64_t seed, uint64_t *nowState, int turns) {
    // バトルエミュを実行してスコアを計算
    (*position) = 1;
    (*nowState) = BattleEmulator::TYPE_2A;
    BattleEmulator::Main(position, turns, genome.actions, players, (std::optional<BattleResult> &) std::nullopt, seed,
                         nullptr, nullptr, -2, nowState);
    // スコア例: ターン数やHP、MPを考慮
    if (players[0].hp <= 0) {
        return -1.0;
    }

    uint8_t state = (*nowState) & 0xf;
    auto previousState = ((*nowState) >> 4) & 0xf;

    auto action = genome.actions[turns - 1];

    if (players[0].mp >= 50 && action == BattleEmulator::ELFIN_ELIXIR || action == BattleEmulator::SAGE_ELIXIR) {
        return -1;
    }


    if (state == BattleEmulator::TYPE_2E && previousState < 5 &&
        action == BattleEmulator::DEFENDING_CHAMPION) {//強攻撃対応
        return 100;
    }

    double point = 0.0;

    if (action == BattleEmulator::DOUBLE_UP && players[0].AtkBuffLevel == 0) {
        point -= 1.5;
    } else if (action == BattleEmulator::ATTACK_ALLY ||
               action == BattleEmulator::MULTITHRUST) {
        point -= 1;
    }

    point += (1 - players[0].ElfinElixirCount);
    point += (3 - players[0].SpecialMedicineCount);
    point += (2 - players[0].SageElixirCount);

    point += (1 - (players[1].hp / players[1].maxHp));
    point += calculateFitness(players[0].hp, players[0].maxHp, 1);
    if (players[0].AtkBuffTurn >= 0 && players[0].BuffTurns <= 2) {
        point += (2 - players[0].BuffTurns) * 0.3;
    } else {
        point += (2 - players[0].BuffLevel);
    }
    if (players[0].AtkBuffTurn >= 0 && players[0].AtkBuffTurn <= 2) {
        point += (2 - players[0].AtkBuffTurn) * 0.3;
    } else {
        point += (2 - players[0].AtkBuffLevel) * 0.5;
    }
    if (players[0].MagicMirrorTurn >= 0 && players[0].MagicMirrorTurn <= 2) {
        point += (2 - players[0].MagicMirrorTurn) * 0.3;
    } else {
        point -= players[0].hasMagicMirror ? 0.5 : 0.0;
    }

    if (action == BattleEmulator::MIDHEAL ||
        action == BattleEmulator::MORE_HEAL ||
        action == BattleEmulator::FULLHEAL) {
        if (players[0].hp / players[0].maxHp > 0.7) {
            point += 3.0; // HPが十分高いとき補助行動をペナルティ
        }
        if (players[1].hp / players[1].maxHp < 0.2) {
            point += 2.0; // 敵が瀕死なら補助行動をペナルティ
        }
    }

    point -= turns * 0.5;


    return 100.0 - point;
    //return  + ((static_cast<int>(((*nowState) >> 12) & 0xfffff) + 1) * 0.125); // ターンが短いほどスコアが高い
}

// 遺伝的アルゴリズム実行
Genome GeneticAlgorithm::RunGeneticAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations,
                                             const int actions[500]) {
    constexpr int populationSize = 1500;
    constexpr double mutationRate = 0.1;
    std::vector<Genome> population(populationSize);

    auto nowState = new uint64_t(0);
    auto position = new int(1);

    // 初期集団生成
    std::mt19937 rng(seed);
    Player CopyPlayers[2];

    for (auto &genome: population) {
        for (int i = 0; i < turns + 1; ++i) {
            if (actions[i] == -1 || actions[i] == 0) {
                genome.actions[i] = actions1[rng() % ACTION_COUNT]; // ランダムに行動を設定
            } else {
                genome.actions[i] = actions[i];
            }
        }
    }
    lcg::init(seed);

    // 世代交代
    for (int generation = 0; generation < maxGenerations; generation++) {

        for (auto &genome: population) {
            for (int i = turns; i < turns + 2; ++i) {
                genome.actions[i] = actions1[rng() % ACTION_COUNT]; // ランダムに行動を設定
            }
        }

        turns += 1;
        for (int i = 0; i < 5; ++i) {

            CopyPlayers[0] = players[0];
            CopyPlayers[1] = players[1];


            // 適応度計算
            for (auto &genome: population) {
                genome.fitness = EvaluateFitness(genome, position, CopyPlayers, seed, nowState, turns);
            }

            // 選択・交叉・突然変異
            std::vector<Genome> nextGeneration;
            std::sort(population.begin(), population.end(), [](const Genome &a, const Genome &b) {
                return a.fitness > b.fitness;
            });

            // 上位個体の選択
            nextGeneration.insert(nextGeneration.end(), population.begin(), population.begin() + 20);

            // 交叉
            while (nextGeneration.size() < populationSize) {
                const auto &parent1 = population[rng() % 10];
                const auto &parent2 = population[rng() % 10];
                Genome child = parent1;
                int crossoverPoint = turns + (rng() % (500 - turns));
                for (int i = crossoverPoint; i < 500; ++i) {
                    child.actions[i] = parent2.actions[i];
                }
                nextGeneration.push_back(child);
            }

            // 突然変異
            for (auto &genome: nextGeneration) {
                if (rng() / double(std::mt19937::max()) < mutationRate) {
                    genome.actions[turns + rng() % (500 - turns)] = actions1[rng() % ACTION_COUNT];
                }
            }

            population = std::move(nextGeneration);
        }
    }

    delete position;
    delete nowState;

    // 最適な遺伝子を返す
    return *std::max_element(population.begin(), population.end(), [](const Genome &a, const Genome &b) {
        return a.fitness < b.fitness;
    });
}