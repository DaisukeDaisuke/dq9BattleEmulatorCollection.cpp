#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <queue>

#include "lcg.h"
#include "BattleEmulator.h"
#include "AnalyzeData.h"
#include "debug.h"
#include "ActionOptimizer.h"

#ifdef DEBUG

#include <chrono>

#endif

int toint(char *string);

//void processResult(const Player *copiedPlayers, const uint64_t seed, std::string input);

std::string ltrim(const std::string &s);

std::string rtrim(const std::string &s);

std::string trim(const std::string &s);

void SearchRequest(const Player copiedPlayers[2], uint64_t seed, const int aActions[350]);

uint64_t BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds, int turns,
                           int eActions[350],
                           int aActions[350], int damages[350]);


void mainLoop(const Player copiedPlayers[2]);

using namespace std;

int foundSeeds = 0;
std::vector<AnalyzeData> analyzeDataMap;

uint64_t FoundSeed = 0;

void printHeader(std::stringstream &ss);

// ヘッダーを出力する関数
void printHeader(std::stringstream &ss) {
    ss << std::left << std::setw(6) << "turn"
            << std::setw(18) << "sp"
            << std::setw(18) << "aAct"
            << std::setw(18) << "eAct1"
            << std::setw(18) << "eAct2"
            << std::setw(6) << "aD"
            << std::setw(6) << "eD1"
            << std::setw(6) << "eD2"
            << std::setw(6) << "ahp"
            << std::setw(6) << "ehp"
            << std::setw(6) << "amp"

            << std::setw(6) << "ini"
            << std::setw(6) << "Para"
            << std::setw(6) << "Sle"
            << std::setw(6) << "DET"
            << std::setw(6) << "POT"
            << std::setw(6) << "BOT"
            << std::setw(6) << "Sct" << "\n";
    ss << std::string(153, '-') << "\n"; // 区切り線を出力
}

std::string dumpTable(BattleResult &result, int32_t gene[350], int PastTurns);

std::string dumpTable(BattleResult &result, int32_t gene[350], int PastTurns) {
    stringstream ss6;
    printHeader(ss6);
    int currentTurn = -1;
    int eDamage[2] = {-1, -1}, aDamage = -1;
    bool initiative_tmp = false;
    std::string eAction[2], aAction, sp, tmpState, ATKTurn1, DEFTurn1, magicMirrorTurn1, specialChargeTurn1, amp1, ahp2,
            ehp2, amp2, poisonTurn1, SpeedTurn1;
    auto counter = 0;
    // データのループ
    for (int i = 0; i < result.position; ++i) {
        auto action = result.actions[i];
        auto damage = result.damages[i];
        auto DEFTurn = result.BuffTurnss[i];
        auto turn = result.turns[i];
        auto initiative = result.initiative[i];
        auto ehp1 = result.ehp[i];
        auto ahp1 = result.ahp[i];
        auto isEnemy = result.isEnemy[i];
        auto state = result.state[i] & 0xf;
        auto specialChargeTurn = result.scTurn[i];
        auto poisonTurn = result.PoisonTurns[i];
        auto SpeedTurn = result.SpeedTurn[i];
        int amp = -1;
        if (i >= 1) {
            amp = result.amp[i - 1];
        }


        if (state == BattleEmulator::TYPE_2A) {
            tmpState = "A";
        } else if (state == BattleEmulator::TYPE_2B) {
            tmpState = "B";
        } else if (state == BattleEmulator::TYPE_2C) {
            tmpState = "C";
        } else if (state == BattleEmulator::TYPE_2D) {
            tmpState = "D";
        }
        if (state == BattleEmulator::TYPE_2E) {
            tmpState = "E";
        }

        auto special = gene[turn];

        std::string specialAction;
        if (special != 0 && special != -1) {
            specialAction = BattleEmulator::getActionName(special & 0x3ff);
        }

        // ターンが変わったら、前のターンのデータを出力
        if (turn != currentTurn) {
            if (currentTurn != -1) {
                // 前のターンの出力
                if (turn > PastTurns) {
                    ss6
                            << std::left << std::setw(6) << (currentTurn + 1)
                            << std::setw(18) << sp
                            << std::setw(18) << aAction
                            << std::setw(18) << eAction[0]
                            << std::setw(18) << eAction[1]
                            << std::setw(6) << aDamage
                            << std::setw(6) << eDamage[0]
                            << std::setw(6) << eDamage[1]
                            << std::setw(6) << ahp2
                            << std::setw(6) << ehp2
                            << std::setw(6) << amp2
                            << std::setw(6) << (initiative_tmp ? "yes" : "")
                            << std::setw(6) << ((aAction == "Paralysis" || aAction == "Cure Paralysis") ? "yes" : "")
                            << std::setw(6) << ((aAction == "Sleeping" || aAction == "Cure Sleeping") ? "yes" : "")
                            << std::setw(6) << DEFTurn1
                            << std::setw(6) << poisonTurn1
                            << std::setw(6) << SpeedTurn1
                            << std::setw(6) << specialChargeTurn1
                            << std::setw(11) << "" << "\n";
                }
            }
            // ターンの初期化
            currentTurn = turn;
            eAction[0] = "";
            eAction[1] = "";
            aAction = "";
            eDamage[0] = 0;
            eDamage[1] = 0;
            aDamage = 0;
            sp = "";
            initiative_tmp = false;
            counter = 0;
            ATKTurn1 = "";
            DEFTurn1 = "";
            poisonTurn1 = "";
            SpeedTurn1 = "";
            magicMirrorTurn1 = "";
            specialChargeTurn1 = "";
        }

        // 敵か味方の行動を適切な変数に格納
        if (isEnemy) {
            eAction[counter] = BattleEmulator::getActionName(action);
            eDamage[counter] = damage;
            counter++;
            ahp2 = std::to_string(ahp1);
        } else {
            ehp2 = std::to_string(ehp1);
            amp2 = std::to_string(amp);
            aAction = BattleEmulator::getActionName(action);
            aDamage = damage;
            if (DEFTurn >= 0) {
                DEFTurn1 = std::to_string(DEFTurn);
            }
            if (poisonTurn >= 0) {
                poisonTurn1 = std::to_string(poisonTurn);
            }

            if (SpeedTurn >= 0) {
                SpeedTurn1 = std::to_string(SpeedTurn);
            }
            if (specialChargeTurn > 0) {
                specialChargeTurn1 = std::to_string(specialChargeTurn);
            }

            amp1 = std::to_string(amp);

            initiative_tmp = initiative;
            sp = specialAction;

            if (eAction[0] != "magic Burst" && eAction[1] != "magic Burst") {
                if (!initiative && action == BattleEmulator::TURN_SKIPPED || action == BattleEmulator::PARALYSIS ||
                    action == BattleEmulator::SLEEPING) {
                    sp = "---------------";
                }
                if ((action == BattleEmulator::CURE_SLEEPING || action == BattleEmulator::CURE_PARALYSIS)) {
                    sp = "---------------";
                }
            }
        }
    }

    // 最後のターンのデータを出力
    if (currentTurn != -1) {
        ss6
                << std::left << std::setw(6) << (currentTurn + 1)
                << std::setw(18) << sp
                << std::setw(18) << aAction
                << std::setw(18) << eAction[0]
                << std::setw(18) << eAction[1]
                << std::setw(6) << aDamage
                << std::setw(6) << eDamage[0]
                << std::setw(6) << eDamage[1]
                << std::setw(6) << ahp2
                << std::setw(6) << ehp2
                << std::setw(6) << amp2
                << std::setw(6) << (initiative_tmp ? "yes" : "")
                << std::setw(6) << ((aAction == "Paralysis" || aAction == "Cure Paralysis") ? "yes" : "")
                << std::setw(6) << ((aAction == "Sleeping") ? "yes" : "")
                << std::setw(6) << DEFTurn1
                << std::setw(6) << poisonTurn1
                << std::setw(6) << SpeedTurn1
                << std::setw(6) << specialChargeTurn1
                << std::setw(11) << "" << "\n";
    }

    return ss6.str();
}

std::string normalDump(AnalyzeData data);

std::string normalDump(AnalyzeData data) {
    int counter = 0;
    BattleResult result = *data.getBattleResult();
    std::stringstream ss;
    for (int i = 0; i < result.position; ++i) {
        auto action = result.actions[i];
        auto damage = result.damages[i];
        if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
            ss << "h ";
            counter++;
        } else if (damage != 0) {
            ss << damage << " ";
        }
        if (counter == 10) {
            ss << std::endl;
            counter = 0;
        }
    }
    auto turn = result.turn + 1;
    if (data.getWinStatus()) {
        ss << "W " << turn;
    } else {
        ss << "L " << turn;
    }
    return ss.str();
}

const std::string version = "v2.0.0";

void showHeader() {
#ifdef BUILD_DATE
    const std::string buildDate = BUILD_DATE;
#else
    const std::string buildDate = "Unknown";
#endif

#ifdef BUILD_TIME
    const std::string buildTime = BUILD_TIME;
#else
    const std::string buildTime = "Unknown";
#endif

    auto compiler = "Unknown";
#if defined(MINGW_BUILD)
    compiler = "mingw";
#elif defined(MSVC_BUILD)
    compiler = "msBuild";
#endif


#if defined(OPTIMIZATION_O3_ENABLED)
    std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (Optimized for O3), Build date: " << buildDate
            << ", " <<
            buildTime << " UTC/GMT, Compiler: " << compiler << std::endl;
#elif defined(OPTIMIZATION_O2_ENABLED)
    std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (Optimized for O2), Build date: " << buildDate << ", " << buildTime  << " UTC/GMT, Compiler: " << compiler << std::endl;
#elif defined(NO_OPTIMIZATION)
    std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (No optimization), Build date: " << buildDate << ", " << buildTime   << " UTC/GMT, Compiler: " << compiler << std::endl;
#else
    std::cout << "dq9 Corvus battle emulator" << version << " (Unknown build configuration), Build date: " << buildDate << ", " << buildTime   << " UTC, Compiler: " << compiler << std::endl;
    << ", " << buildTime << std::endl;
#endif
}


//int main(int argc, char *argv[]) {
int main() {
    showHeader();
#ifdef DEBUG
    auto t0 = std::chrono::high_resolution_clock::now();
#endif


    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    //std::cin.tie(0)->sync_with_stdio(0);

    const Player copiedPlayers[2] = {
        // プレイヤー1
        {
            103, 103.0, 89, 89, 97, 97, 69, 69, 44, 36, // 最初のメンバー
            36, false, false, 0, false, 0, -1,
            // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
            6, 1.0, false, -1, 0, -1, // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
            false, -1, 0, -1, 0, false, 1, 1, 1, -1, 0, -1, false, 2, false, -1
        }, // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel

        // プレイヤー2
        {
            696, 696.0, 68, 68, 68, 68, 50, 50, 0, 255, // 最初のメンバー
            255, false, false, 0, false, 0, -1,
            // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
            0, 1.0, false, -1, 0, -1, // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
            false, -1, 0, -1, 0, false, 0, 0, 0, -1, 0, -1, false, 2, false, -1
        } // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel
    };


#ifdef DEBUG2
    //time1 = 0x199114b2;
    //time1 = 0x226d97a6;
    //time1 = 0x1c2a9bda;
    //time1 = 0x1aa6c05d;
    uint64_t time1 = 0x24588ee6;

    int dummy[100];
    lcg::init(time1);
    int *position1 = new int(1);

    //0x24588ee6: 25, 25, 54, 55, 54, 54, 25, 54, 25, 54, 54, 54, 25, 50, 56, 50, 25, 54,
    /*
        *NowStateの各ビットの使用状況は下記の通りである。
        +-+-+-+-+-+-+-+-+- (* NowState) -+-+-+-+-+-+-+-+-+
           |            Name            |     size      |
        0  | Current Rotation Table     |     4bit      |
        4  | Rotation Internal State    |     4bit      |
        8  | Free Camera State          |     4bit      |
        12 | Turn Count Processed       |     20bit     |
        32 | Combo Previous Attack Id   |     2byte     |
        40 | Combo Counter              |     1byte     |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                     合計 6Byte
    */

    auto *NowState = new uint64_t(0); //エミュレーターの内部ステートを表すint

    Player players1[2];
    //int32_t gene1[350] = {0};
    int32_t gene1[350] = {25, 25, 54, 55, 54, 54, 25, 54, 25, 54, 54, 54, 25, 50, 56, 50, 25, 54};
    //gene1[19-1] = BattleEmulator::DEFENCE;
    int counter = 0;

    // gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    // gene1[counter++] = BattleEmulator::FLEE_ALLY;
    // gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    // gene1[counter++] = BattleEmulator::MIRACLE_SLASH;

    //for (int i = 0; i < 10; ++i) {
    (*NowState) = 0;
    (*position1) = 1;
    std::optional<BattleResult> dummy1;
    dummy1 = BattleResult();
    std::memcpy(players1, copiedPlayers, sizeof(players1));
    BattleEmulator::Main(position1, (counter == 0 ? 1000 : counter), gene1, players1, dummy1, time1, dummy, dummy, -1,
                         NowState);

    std::stringstream ss1;
    ss1 << time1 << " ";

    if (dummy1.has_value()) {
        std::cout << dumpTable(dummy1.value(), gene1, -1) << std::endl;
    }
    //}
    delete position1;
    delete NowState;

    return 0;
#endif

#ifdef DEBUG3
    uint64_t time1 = 0x24588ee6;

    int actions[350] = {
        BattleEmulator::ATTACK_ALLY,
        -1,
    };
    SearchRequest(copiedPlayers, time1, actions);

    return 0;
#endif

    mainLoop(copiedPlayers);
    return 0;

#ifdef DEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
#endif
    return 0;
}

constexpr int32_t actions1[100] = {
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

void SearchRequest(const Player copiedPlayers[2], uint64_t seed, const int aActions[350]) {
#ifdef DEBUG
    auto t0 = std::chrono::high_resolution_clock::now();
    BattleEmulator::ResetTurnProcessed();
#endif

    int32_t gene[350] = {0};
    auto turns = 0;
    for (int i = 0; i < 350; ++i) {
        gene[i] = aActions[i];
        if (aActions[i] == -1) {
            gene[i] = -1;
            gene[i + 1] = -1;
            break;
        }
        turns++;
    }

    lcg::init(seed);

    BattleResult bestResult;
    Genome bestGenome;
    int maxTurns = INT_MAX - 1;

    priority_queue<Genome> que;

    std::optional<BattleResult> result1;
    result1 = BattleResult();
    Player players[2];

    auto *position = new int(1);
    auto *nowState = new uint64_t(0);

    for (int i = 0; i < 1500; ++i) {
        auto genome = ActionOptimizer::RunAlgorithm(copiedPlayers, seed, turns, 1500, gene, i * 2);

        players[0] = copiedPlayers[0];
        players[1] = copiedPlayers[1];

        (*position) = 1;
        (*nowState) = 0;

        result1->clear();
        BattleEmulator::Main(position, turns + 100, genome.actions, players, result1, seed, nullptr, nullptr, -1,
                             nowState);

        if (players[0].hp >= 0 && players[1].hp == 0) {
            if (result1->turn < maxTurns) {
                maxTurns = result1->turn;
                bestResult = result1.value();
                bestGenome = genome;
            }
        }
    }


    delete position;
    delete nowState;

    std::cout << dumpTable(bestResult, bestGenome.actions, 0) << std::endl;

    std::cout << "0x" << std::hex << seed << std::dec << ": ";

    for (auto i = 0; i < 100; ++i) {
        if (bestGenome.actions[i] == 0 || bestGenome.actions[i] == -1) {
            break;
        }
        std::cout << bestGenome.actions[i] << ", ";
    }
    std::cout << std::endl;

#ifdef DEBUG
    auto t3 = std::chrono::high_resolution_clock::now();
    auto elapsed_time1 =
            std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time1) / 1000 << " ms" << std::endl;
    std::cout << "Searcher Turn Consumed: " << BattleEmulator::getTurnProcessed() << " (" << (
        static_cast<double>(BattleEmulator::getTurnProcessed()) / 10000) << " mann)" << std::endl;
    // 1秒あたりの探索回数 (万回.?? 形式)
    // 正しい計算：1秒あたりの探索回数 (万回/秒)
    double performance = (static_cast<double>(BattleEmulator::getTurnProcessed()) * 100.0) /
                         static_cast<double>(elapsed_time1);
    std::cout << "Performance: " << std::fixed << std::setprecision(2) << performance << " mann turns/s" << std::endl;
#endif
}

// ブルートフォースリクエスト関数
[[nodiscard]] uint64_t BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds, int turns,
                                         int eActions[350],
                                         int aActions[350], int damages[350]) {
    std::cout << "BruteForceRequest executed with time " << hours << ":" << minutes << ":" << seconds << std::endl;
    std::cout << "eActions: ";
    for (int i = 0; i < 350 && eActions[i] != -1; ++i) std::cout << eActions[i] << " ";
    std::cout << "\naActions: ";
    for (int i = 0; i < 350 && aActions[i] != -1; ++i) std::cout << aActions[i] << " ";
    std::cout << "\ndamages: ";
    for (int i = 0; i < 350 && damages[i] != -1; ++i) std::cout << damages[i] << " ";
    std::cout << std::endl;

    foundSeeds = 0;
    FoundSeed = 0;

    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    totalSeconds = totalSeconds - 17;
    //std::cout << totalSeconds << std::endl;
    auto time1 = static_cast<uint64_t>(floor((totalSeconds - 30) * (1 / 0.12515)));
    time1 = time1 << 16;
    std::cout << time1 << std::endl;

    auto time2 = static_cast<uint64_t>(floor((totalSeconds + 30) * (1 / 0.125155)));
    time2 = time2 << 16;
    std::cout << time2 << std::endl;
    ////
    //    time1 = 0;
    //    time2 = 4294967296;
    ////    time1 = 0x04b8d631 - 100000;
    ////    time2 = 0x04b8d631 + 100000;
    //
    int32_t gene[350] = {0};
    for (int i = 0; i < 350; ++i) {
        gene[i] = aActions[i];
        if (aActions[i] == -1) {
            gene[i] = -1;
            gene[i + 1] = -1;
            break;
        }
    }

    /*
    *NowStateの各ビットの使用状況は下記の通りである。
    +-+-+-+-+-+-+-+-+- (* NowState) -+-+-+-+-+-+-+-+-+
       |            Name            |     size      |
    0  | Current Rotation Table     |     4bit      |
    4  | Rotation Internal State    |     4bit      |
    8  | Free Camera State          |     4bit      |
    12 | Turn Count Processed       |     20bit     |
    32 | Combo Previous Attack Id   |     2byte     |
    40 | Combo Counter              |     1byte     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                 合計 6Byte
    */
    int *position = new int(1);
    auto *nowState = new uint64_t(0);
    int maxElement = 350;
    Player players[2];
    for (uint64_t seed = time1; seed < time2; ++seed) {
        //        if (seed % 1000000000 == 0) {
        //            std::cout << seed << std::endl;
        //        }
        lcg::init(seed);
        // for (int st = BattleEmulator::TYPE_2A; st < BattleEmulator::TYPE_2D; ++st) {
        (*nowState) = BattleEmulator::TYPE_2A;
        (*position) = 1;
        //std::memcpy(players, copiedPlayers, sizeof(players));
        players[0] = copiedPlayers[0];
        players[1] = copiedPlayers[1];


        bool resultBool = BattleEmulator::Main(position, turns, gene, players,
                                               (optional<BattleResult> &) std::nullopt, seed, eActions, damages,
                                               maxElement,
                                               nowState);
        if (resultBool) {
            //std::cout << seed << ", " << st << std::endl;
            std::cout << seed << std::endl;
            FoundSeed = seed;
            foundSeeds++;
        }
        //}
    }
    delete position;
    delete nowState;

    std::cout << std::endl << "found: " << foundSeeds << std::endl;

    if (foundSeeds == 1) {
        return FoundSeed;
    }
    if (foundSeeds == 0) {
        std::cout << "not found!!!" << std::endl;
        return 0;
    }
    FoundSeed = 0;
    foundSeeds = 0;
    return 0;
}

// 入力文字列を配列に分割するヘルパー関数
void parseActions(const std::string &str, int actions[350]) {
    std::istringstream iss(str);
    int value, index = 0;
    while (iss >> value && index < 350) {
        actions[index++] = value;
    }
    actions[index++] = -1;
    actions[index++] = -1;
    actions[index++] = -1;
}


// メインループ
void mainLoop(const Player copiedPlayers[2]) {
    int eActions[350] = {0};
    int aActions[350] = {0};
    int damages[350] = {0};

    std::string input;
    while (std::getline(std::cin, input)) {
        //意図せずcinが閉じられると無限ループするので対策
        if (input.empty()) continue;

        char command = input[0];
        if (command == 'q') {
            std::cout << "Exiting loop." << std::endl;
            return;
        }
        if (command == 'b') {
            // Check if there is enough input (e.g., at least "b " and some parameters)
            if (input.size() < 3) {
                std::cerr << "Error: insufficient input for command 'b'." << std::endl;
                continue;
            }

            // Extract the substring after the command character and a space
            std::string params = input.substr(2);
            if (params.empty()) {
                std::cerr << "Error: no parameters provided for command 'b'." << std::endl;
                continue;
            }

            std::istringstream ss(params);

            int hours, minutes, seconds, turns;
            if (!(ss >> hours >> minutes >> seconds >> turns)) {
                std::cerr << "Error: failed to parse time parameters." << std::endl;
                continue;
            }

            // Read the three action strings separated by '-' delimiters
            std::string eActionsStr, aActionsStr, damagesStr;
            if (!std::getline(ss, eActionsStr, '-')) {
                std::cerr << "Error: failed to read eActions." << std::endl;
                continue;
            }
            if (!std::getline(ss, aActionsStr, '-')) {
                std::cerr << "Error: failed to read aActions." << std::endl;
                continue;
            }
            if (!std::getline(ss, damagesStr, '-')) {
                std::cerr << "Error: failed to read damages." << std::endl;
                continue;
            }

            // 各アクション配列に値を代入
            parseActions(eActionsStr, eActions);
            parseActions(aActionsStr, aActions);
            parseActions(damagesStr, damages);

            auto seed = BruteForceRequest(copiedPlayers, hours, minutes, seconds, turns, eActions, aActions, damages);
            if (foundSeeds == 1) {
                SearchRequest(copiedPlayers, seed, aActions);
            }
            continue;
        }
        if (command == 'h') {
            showHeader();
            continue;
        }
        std::cerr << "Unknown command." << std::endl;
    }
    if (std::cerr.good()) {
        std::cerr <<
                "Unrecoverable Error: An anomaly occurred in the main loop of the C++ process, forcing the battle emulator process to terminate. To recover, please restart the integrated system"
                << std::endl;
    }
}

//void processResult(const Player *copiedPlayers, const uint64_t seed,
//                   const std::string input) {
//
//    return;
//}

int toint(char *str) {
    try {
        int number = std::stoi(str);
        return number;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return -1;
    } catch (const std::out_of_range &e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return -1;
    }
}


// 左側の空白をトリム
std::string ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : s.substr(start);
}

// 右側の空白をトリム
std::string rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// 両側の空白をトリム
std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}
