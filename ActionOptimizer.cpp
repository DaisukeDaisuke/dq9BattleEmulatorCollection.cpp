//
// Created by Owner on 2024/11/21.
//

#include "ActionOptimizer.h"
#include <vector>
#include <random>
//#include <functional>
#if defined(MULTITHREADING)
#include <future>
#include "lcg.h"
#endif
//#include <queue>
#include "BattleEmulator.h"
#include "Genome.h"
#include "ActionBanManager.h"
#include "HeapQueue.h"

void ActionOptimizer::updateCompromiseScore(Genome &genome) {
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

#if defined(MULTITHREADING)

/**
 * @brief 各ワーカースレッドで実行されるメイン処理
 *
 * この関数はマルチスレッド環境下で RunAlgorithmAsync から呼ばれるワーカー処理として設計されています。
 * シングルスレッドでアルゴリズムを実行する場合は、RunAlgorithm を使用してください。
 * 指定された範囲内で最適なターン数およびゲノムを探索し、その結果をペアとして返します。
 *
 * @param players 味方と敵の2つの Player オブジェクトを含む配列。players[0] が味方、players[1] が敵です。
 * @param seed    バトルの初期シード値。乱数生成の際のシードとして利用されます。
 * @param turns   現在のターン数（探索開始の基準）。
 * @param maxGenerations マルチスレッドにおける総反復回数。
 * @param actions アルゴリズムが利用するアクションの履歴。（サイズは 350 と仮定）
 * @param start   アルゴリズムが探索を開始するインデックスを指定します。
 * @param end     アルゴリズムが探索を終了するインデックスを指定します。
 * @return        探索で処理されたターン数と最適化されたゲノムのペア
 */

std::pair<int, Genome> ActionOptimizer::RunAlgorithmSingleThread(const Player players[2], uint64_t seed, int turns,
                                                                 int maxGenerations, int actions[], int start,
                                                                 int end) {
    BattleEmulator::ResetTurnProcessed();
    auto seed1 = seed;
    auto turns1 = turns;
    auto maxGenerations1 = maxGenerations;

    Genome bestGenome = {};
    bestGenome.turn = INT_MAX;

    std::unique_ptr<int> position = std::make_unique<int>(1);
    std::unique_ptr<uint64_t> nowState = std::make_unique<uint64_t>(0);

    std::optional<BattleResult> result1;
    result1 = BattleResult();

    for (int i = start; i < end; ++i) {
        Genome candidate = RunAlgorithm(players, seed1, turns1, maxGenerations1, actions, i * 2);
        Player localPlayers1[2] = {players[0], players[1]};

        *position = 1;
        *nowState = 0;

        result1->clear();
        BattleEmulator::Main(position.get(), 100, candidate.actions, localPlayers1, result1, seed, nullptr, nullptr, -1,
                             nowState.get());

        if (localPlayers1[0].hp >= 0 && localPlayers1[1].hp == 0) {
            candidate.turn = result1->turn;
            if (candidate.turn < bestGenome.turn) {
                candidate.AllyPlayer.hp = localPlayers1[0].hp;
                candidate.EnemyPlayer.hp = localPlayers1[1].hp;
                bestGenome = candidate;
            }
        }
    }
    // TurnProcessedを取得
    int turnProcessed = BattleEmulator::getTurnProcessed();

    // turnProcessedとbestGenomeをペアで返す
    return std::make_pair(turnProcessed, bestGenome);
}


/**
 * マルチスレッドを利用して、プレイヤーの状態に基づく行動最適化アルゴリズムを非同期的に実行します。
 * 各スレッドは異なる探索範囲を担当し、ターンシミュレーションの最適な候補を並列的に計算します。
 * 全スレッドの計算結果から最適なターン数とゲノムを選択して返します。
 *
 * @param players       味方と敵の2つの Player オブジェクトが格納された配列です。
 *                      players[0] が味方、players[1] が敵に対応します。
 * @param seed          バトルの実際の初期シードかつ、乱数生成器のシード値です。同じシードを指定すると計算結果の再現性が保たれます。
 * @param turns         ゲームの現在のターン数を示します。探索開始位置として利用されます。
 * @param totalIterations  全体で探索を繰り返す回数（イテレーション数）です。探索規模を設定します。
 * @param actions       既存の過去行動が格納された配列です。配列の長さは350である必要があります。
 *                      配列内の値 -1 または 0 は有効な終了を示します。
 * @param numThreads    同時にスレッドを使用して並列処理を行うスレッドの数を指定します。
 *
 * @return              計算終了後に処理した総ターン数と最適化されたゲノム情報を格納したペアを返します。
 */
std::pair<int, Genome> ActionOptimizer::RunAlgorithmAsync(const Player players[2], uint64_t seed, int turns,
                                                          int totalIterations, int actions[350], int numThreads) {
    lcg::init(seed, true);
    int chunkSize = totalIterations / numThreads;

    std::vector<std::future<std::pair<int, Genome> > > futures;
    futures.reserve(numThreads); // ✅ メモリ確保でスコープ外の問題を防ぐ

    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? totalIterations : start + chunkSize;

#if defined(BattleEmulatorLV13)
        futures.push_back(std::async(std::launch::async, RunAlgorithmSingleThread,
                                     std::cref(players), seed, turns, 2000, actions, start, end));
#elif defined(BattleEmulatorLV19)
        futures.push_back(std::async(std::launch::async, RunAlgorithmSingleThread,
                                             std::cref(players), seed, turns, 1500, actions, start, end));
#elif defined(BattleEmulatorLV15)
        futures.push_back(std::async(std::launch::async, RunAlgorithmSingleThread,
                                     std::cref(players), seed, turns, 1500, actions, start, end));
#endif
    }

    Genome bestGenome = {};
    bestGenome.turn = INT_MAX;

    int totalTurnProcessed = 0;

    for (auto &future: futures) {
        auto [turnProcessed, candidate] = future.get(); // ペアを取得
        totalTurnProcessed += turnProcessed;
        if (candidate.turn < bestGenome.turn) {
            bestGenome = candidate;
        }
    }

    return std::make_pair(totalTurnProcessed, bestGenome);
}

#endif

// オレオレアルゴリズム実行

/**
 * プレイヤーの状態情報を基に、最適な行動方針を決定するためのアルゴリズムによる最適化処理を実行します。
 * このアルゴリズムは、ヒューリスティックな適応度評価に基づいて反復的に行動候補を探索するために
 * 優先度付きキューを利用し、ターン制バトルシミュレーションにおける最適な決定をシミュレートすることを目指します。
 *
 * @param players  味方と敵の2つの Player オブジェクトが格納された配列です。
 *                 players[0] が味方、players[1] が敵に対応します。
 * @param seed     乱数生成用のシード値です。同じシードを使用することで再現性が保証されます。
 * @param turns    ゲームの進行状態を示す正確なターン数です。探索開始位置として利用されます。
 * @param maxGenerations 最適化処理（世代/反復）の最大回数です（-1の場合は制限なしとなります）。
 * @param actions  最適化のすでに行動済みの過去行動が格納された配列です。サイズは350でなければなりません。
 *                 配列内の値 -1 または 0 は有効な行動の終了を示します。
 * @param seedOffset アルゴリズム(乱数生成)の動作を一貫させるために、主シードに加算されるオフセット値です。
 *
 * @return 遺伝子（Genome）オブジェクトを返します。これはアルゴリズムにより決定された状態と最適行動戦略を
 *         含み、適応度、行動履歴、ターン履歴など詳細情報を持っています。
 *
 * この関数は、指定された seed と seedOffset を用いて乱数生成器を初期化し、
 * シミュレーション中の確率的挙動を駆動します。優先度付きキューを活用し、ヒューリスティック
 * に基づいて候補解を生成・進化させていきます。このキューは、各反復で評価される適応度スコアを基準
 * に解の優先順位付けを行います。
 *
 * アルゴリズムの処理としては以下を行います：
 * - プレイヤーの状態をコピーし、シミュレーションが進行するにつれて更新します。
 * - 各行動の適応度に与える影響を評価します。
 * - 行動禁止などのルールに従い、冗長または好ましくない状態を追跡・回避します。
 * - 優先度付きキューを利用して候補となる Genome を保存・比較し、さらなる探索のために
 *   潜在的な最適解を効率的に選択します。
 * - 指定された場合、探索する解の世代数に上限を設けることで計算量を制約します。
 *
 * 特に取り扱うケースは以下の通りです：
 * - 睡眠、麻痺、ターンスキップなどの望ましくない状態に対する適応度の調整。
 * - 毒の軽減や関連するバフ/デバフに対する解毒アイテムなど、特定行動の動的な取り扱い。
 *
 * 前提条件：
 * - Player オブジェクト、行動配列、その他補助構造体は正しく初期化されていること。
 * - actions 配列のサイズは正確に350であること。
 * - 乱数生成器 (RNG) は十分な確率シナリオをモデル化できること。
 *
 * 制約事項：
 * - 状態と遺伝子の処理のために十分なメモリ領域が必要です。
 * - 実行時間は maxGenerations およびプレイヤーの状態や行動の複雑さに依存します。
 * - maxGenerations に制限が厳しい場合、最適解が必ずしも見つかるとは限りません。
 *
 * 本関数は、主にdq9における戦闘のゲーム最適化において、条件に基づきターン制の意思決定を
 * 反復的に最適化する必要がある際に利用されることを意図しています。
 */
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


        Player CopedPlayers[2] = {currentGenome.AllyPlayer, currentGenome.EnemyPlayer};
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

        if (tmpgenomu.Initialized && (currentGenome.EActions[0] == BattleEmulator::KASAP || currentGenome.EActions[1] ==
                                      BattleEmulator::KASAP) && currentGenome.AllyPlayer.BuffLevel != 0) {
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
                currentGenome.actions[turns] = BattleEmulator::SPECIAL_MEDICINE; // -1消しとかないとなぜかズレる
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
        const auto AllyPlayer = CopedPlayers[0];
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
            currentGenome.actions[turns - 1] = action; //-1 消すとなぜか爆速になるが性能は落ちる

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

#if defined(BattleEmulatorLV13)
            if (visited == 0 && ehp1 - currentGenome.EnemyPlayer.hp > 45) {
                currentGenome.fitness += 60;
            }
#elif defined(BattleEmulatorLV19)
            if (visited == 0 && ehp1 - currentGenome.EnemyPlayer.hp > 70) {
                currentGenome.fitness += 30;
            }
#elif defined(BattleEmulatorLV15)
            if (visited == 0 && ehp1 - currentGenome.EnemyPlayer.hp > 45) {
                currentGenome.fitness += 60;
            }
#endif


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

#ifdef BattleEmulatorLV19

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
#endif
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

        if (AllyPlayerPre.specialCharge == true && AllyPlayerPre.specialChargeTurn != 0 && AllyPlayerPre.acrobaticStar
            == false && !Bans.is_action_banned(
                BattleEmulator::ACROBATIC_STAR, turns)) {
            action = BattleEmulator::ACROBATIC_STAR;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
#if defined(BattleEmulatorLV13)
                currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 13);
#elif defined(BattleEmulatorLV19)
                currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 13);
#elif defined(BattleEmulatorLV15)
                currentGenome.fitness = baseFitness + 10 + static_cast<int>(rng() % 13);
#endif
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

#if defined(BattleEmulatorLV13) || defined(BattleEmulatorLV15)
        if (AllyPlayerPre.mp >= 3 && !Bans.is_action_banned(BattleEmulator::WOOSH_ALLY, turns)) {
            action = BattleEmulator::WOOSH_ALLY;
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
        if (!Bans.is_action_banned(BattleEmulator::DRAGON_SLASH, turns)) {
            action = BattleEmulator::DRAGON_SLASH;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 5 + static_cast<int>(rng() % 6);
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
        /*if (!Bans.is_action_banned(BattleEmulator::ITEM_USE, turns)) {
            action = BattleEmulator::ITEM_USE;
            if (tmpgenomu.Visited >= 1) {
                currentGenome.fitness = baseFitness; // 固定値に
                currentGenome.Visited = 0;
            } else {
                currentGenome.fitness = baseFitness + 2 + static_cast<int>(rng() % 6);
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
        }*/
#endif

#ifdef BattleEmulatorLV19

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
#endif

        counter++;
    }

    if (found) {
        return BaseGenome;
    }
    Genome bestGenome = que.empty() ? currentGenome : static_cast<Genome>(que.top());
    return bestGenome;
}
