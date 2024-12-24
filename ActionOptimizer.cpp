//
// Created by Owner on 2024/11/21.
//

#include "ActionOptimizer.h"
#include <vector>
#include <random>
#include <functional>
#include <queue>
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

void processBattleResult(const std::optional<BattleResult> &result, Genome &genome) {
    // resultが有効かチェック
    if (!result.has_value()) return;

    int counter1 = 0;

    // 指定された範囲内で処理
    for (int i = result->position - 3; i < result->position; ++i) {
        if (result->isEnemy[i]) {
            genome.EActions[counter1] = result->actions[i];
            genome.Edamage[counter1++] = result->damages[i];
        } else {
            genome.Aactions = result->actions[i];
        }

        // カウンタの上限チェック（範囲外アクセス防止）
        if (counter1 >= 2) break;
    }
}


int evaluateActionPenalty(const Genome &currentGenome, bool Initialized) {
    int penalty = 0;


    if (!Initialized) {
        // 初期化されていない場合、探索開始数ターンなのでペナルティなし
        return penalty; // ペナルティなし
    }

    // 休みになった場合のペナルティ処理
    if (currentGenome.Aactions == BattleEmulator::INACTIVE_ALLY) {
        // 初期化されている場合はペナルティを加算
        penalty -= 5; // 休みの場合の基準ペナルティ
    }

    // 魔人切り（Hatchet Man）が当たった場合のペナルティ処理
    if ((currentGenome.EActions[0] == BattleEmulator::HATCHET_MAN && currentGenome.Edamage[0] > 0) ||
        (currentGenome.EActions[1] == BattleEmulator::HATCHET_MAN && currentGenome.Edamage[1] > 0)) {
        // 魔人切りが命中した場合
        penalty -= 10; // ペナルティを減らす（優先度を上げる）
    }

    // 必要なら他の条件をここに追加し、ペナルティを調整

    if (currentGenome.Aactions == BattleEmulator::MULTITHRUST) {
        penalty += 1;
    }

    return penalty;
}


bool executeAction(int actionType, Genome &currentGenome, Genome &tmpgenomu,
                   int Fitness, Player *CopedPlayers,
                   int *position, uint64_t *nowState) {
    // 現在の遺伝子が既に訪問された場合
    if (currentGenome.Visited >= 1) {
        // 訪問フラグをリセットし、フィットネス値を固定値に設定
        currentGenome.fitness = tmpgenomu.fitness; // 固定値に
        currentGenome.Visited = 0;
    } else {
        // 訪問されていない場合、累積フィットネス値に加算
        currentGenome.fitness = tmpgenomu.fitness + Fitness;
    }

    // 現在のターン番号を基に、対応するアクションを設定
    // ターンは1から始まるため、配列のインデックスと一致させるには-1が必要
    currentGenome.actions[tmpgenomu.turn - 1] = actionType;

    // 現在のプレイヤー情報をコピー
    CopedPlayers[0] = tmpgenomu.AllyPlayer; // 味方プレイヤー
    CopedPlayers[1] = tmpgenomu.EnemyPlayer; // 敵プレイヤー

    // 現在の位置と状態を設定
    (*position) = tmpgenomu.position;
    (*nowState) = tmpgenomu.state;

    // 戦闘結果を格納するオプション型の変数を初期化
    std::optional<BattleResult> result = BattleResult();

    // 戦闘シミュレーションを実行
    // "processed"は既に処理済みのターンを示す
    BattleEmulator::Main(position, tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                         CopedPlayers,
                         result, 0ll,
                         nullptr, nullptr, -1, nowState);

    // シミュレーション後、現在の状態を更新
    currentGenome.position = (*position);
    currentGenome.state = (*nowState);
    currentGenome.turn = tmpgenomu.turn + 1; // 次のターンに進む
    currentGenome.processed = tmpgenomu.turn; // 処理済みターンを更新
    currentGenome.AllyPlayer = CopedPlayers[0]; // 味方プレイヤー情報を更新
    currentGenome.EnemyPlayer = CopedPlayers[1]; // 敵プレイヤー情報を更新

    // 戦闘結果を処理
    processBattleResult(result, currentGenome);

    //負けたか勝った時
    if (CopedPlayers[0].hp <= 0 || CopedPlayers[1].hp <= 0) {
        return false;
    }

    currentGenome.fitness += evaluateActionPenalty(currentGenome, tmpgenomu.Initialized);

    // 常にtrueを返す（エラーハンドリングが必要なら条件を追加）
    return true;
}


// オレオレアルゴリズム実行
Genome ActionOptimizer::RunAlgorithm(const Player players[2], uint64_t seed, int turns, int maxGenerations,
                                     int actions[350], int seedOffset) {
    std::mt19937 rng(seed + seedOffset);
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
    genome.Edamage[0] = -1;
    genome.Edamage[1] = -1;
    genome.Aactions = -1;
    genome.fitness = 0;
    genome.turn = turns;
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

    CopedPlayers[0] = genome.AllyPlayer;
    CopedPlayers[1] = genome.EnemyPlayer;
    (*position) = genome.position;
    (*nowState) = genome.state;

    std::optional<BattleResult> result;
    result = BattleResult();
    BattleEmulator::Main(position, genome.turn - genome.processed, genome.actions,
                         CopedPlayers,
                         result, seed,
                         nullptr, nullptr, -1, nowState);

    genome.AllyPlayer = CopedPlayers[0];
    genome.EnemyPlayer = CopedPlayers[1];
    genome.position = (*position);
    genome.state = (*nowState);
    genome.turn = turns + 1;
    genome.processed = turns;

    processBattleResult(result, genome);

    que.push(genome);

    Genome tmpgenomu;
    Genome currentGenome;


    while (!que.empty() && (maxGenerations == -1 || maxGenerations > counter)) {
        currentGenome = que.top();
        que.pop();
        tmpgenomu = currentGenome;

        turns = currentGenome.turn;

        Bans.ban_action(turns, currentGenome.actions[turns - 2]);

        currentGenome.Initialized = true;
        auto AllyPlayerPre = currentGenome.AllyPlayer;
        auto baseFitness = currentGenome.fitness;
        //if (!skip) {

        if (AllyPlayerPre.mp >= 20) {
            if (AllyPlayerPre.BuffLevel <= 1 && !Bans.is_action_banned(BattleEmulator::BUFF, turns)) {
                if (executeAction(BattleEmulator::BUFF, currentGenome, tmpgenomu, 9 + (rng() % 15), CopedPlayers,
                                  position,
                                  nowState)) {
                    que.push(currentGenome);
                }
            }

            if (!Bans.is_action_banned(BattleEmulator::DEFENDING_CHAMPION, turns)) {
                if (executeAction(BattleEmulator::DEFENDING_CHAMPION, currentGenome, tmpgenomu, 1 + (rng() % 8),
                                  CopedPlayers, position, nowState)) {
                    que.push(currentGenome);
                                  }
            }

            if (!Bans.is_action_banned(BattleEmulator::MORE_HEAL, turns) && (AllyPlayerPre.hp / AllyPlayerPre.maxHp) <
                0.7) {
                if (executeAction(BattleEmulator::MORE_HEAL, currentGenome, tmpgenomu, 6 + (rng() % 8), CopedPlayers,
                                  position, nowState)) {
                    que.push(currentGenome);
                }
            }

            if (!Bans.is_action_banned(BattleEmulator::MIDHEAL, turns) && (AllyPlayerPre.hp / AllyPlayerPre.maxHp) <
                0.7) {
                if (executeAction(BattleEmulator::MIDHEAL, currentGenome, tmpgenomu, 6 + (rng() % 8), CopedPlayers,
                                  position, nowState)) {
                    que.push(currentGenome);
                }
            }

            if (!Bans.is_action_banned(BattleEmulator::MULTITHRUST, turns)) {
                if (executeAction(BattleEmulator::MULTITHRUST, currentGenome, tmpgenomu, 15 + (rng() % 8),
                                  CopedPlayers,
                                  position, nowState)) {
                    que.push(currentGenome);
                }
            }


            if (AllyPlayerPre.AtkBuffLevel == 0 && AllyPlayerPre.BuffLevel == 2 && !Bans.is_action_banned(
                    BattleEmulator::DOUBLE_UP, turns)) {
                if (executeAction(BattleEmulator::DOUBLE_UP, currentGenome, tmpgenomu, 20 + (rng() % 8), CopedPlayers,
                                  position, nowState)) {
                    que.push(currentGenome);
                }
            }
        }
        if (!Bans.is_action_banned(BattleEmulator::ATTACK_ALLY, turns)) {
            if (executeAction(BattleEmulator::ATTACK_ALLY, currentGenome, tmpgenomu, 10 + (rng() % 8), CopedPlayers,
                              position, nowState)) {
                que.push(currentGenome);
                              }
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
