#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "lcg.h"
#include "BattleEmulator.h"
#include "debug.h"
#include "ActionOptimizer.h"
#include "InputBuilder.h"

#ifdef DEBUG

#include <chrono>

#endif


namespace {
    // MinGW/GCC用のnoinline属性
#ifndef NOINLINE
#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif
#endif

    int toint(char *string);

    //void processResult(const Player *copiedPlayers, const uint64_t seed, std::string input);

    std::string ltrim(const std::string &s);

    std::string rtrim(const std::string &s);

    std::string trim(const char *s);

    std::string trim(const std::string &s);

    bool isMatchStrWithTrim(const char *s1, const char *s2);

    bool EasterEgg(int argc, char *argv[]);

    void help(const char *program_name);

    void SearchRequest(const Player copiedPlayers[2], uint64_t seed, const int aActions[350], int numThreads);

    uint64_t BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds, int turns,
                               int aActions[350], int damages[350]);

    void dumpTableMain(BattleResult &result1, Genome &genome, uint64_t seed, int turns);

    void printHeader(std::stringstream &ss);

    int foundSeeds = 0;

    uint64_t FoundSeed = 0;

    const char *version = "v2.0.13";

    std::stringstream performanceLogger = std::stringstream();

    constexpr int THREAD_COUNT = 4;
    // `InputBuilder` インスタンス作成
    InputBuilder builder;

    const char repoURL[] =
            u8"Hello Analyzer!, here is the repository URL: https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/bilyouma !branch: bilyouma";
    const char explanation1[] =
            u8"This is part of the dq9 battle emulator and consists of the argument decoder(main and InputBuilder), the brute force attack code (main BruteForceRequest), the action optimization (main::SearchRequest) code, and the battle emulator (BattleEmulator::Main).";

#if defined(MULTITHREADING)
    const char explanation2[] =
            u8"multithreading is enabled, Some functions have been added to ActionOptimizer, and SearchRequest has been modified for multi-threading.";
#elif defined(NO_MULTITHREADING)
    const char explanation2[] = u8", multithreading is disabled";
#endif
    const char explanation3[] =
            u8"2024-2025 DaisukeDaisuke, For all the dq9 solo runners, MIT License, Open Source Freeware, Good luck to all runners in breaking the 8 hour mark for the dq9 solo travel RTA! (still unachieved as of 3/25/2025)";
    const char explanation4[] = u8"Have fun exploring the artists!";

#ifdef HAGANE

    constexpr Player BasePlayers[2] = {
        // プレイヤー1
        {
            107, 107.0, 113, 113, 100, 100, 72, 72, 46, 38, // 最初のメンバー
            38, false, false, 0, false, 0, -1,
            // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
            6, 1.0, false, -1, 0, -1, // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
            false, -1, 0, -1, 0, false, 1, 1, 1, -1, 0, -1, false, 2, false, -1
        }, // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel

        // プレイヤー2
        {
            796, 796.0, 80, 80, 78, 78, 56, 56, 0, 255, // 最初のメンバー
            255, false, false, 0, false, 0, -1,
            // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
            0, 1.0, false, -1, 0, -1, // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
            false, -1, 0, -1, 0, false, 0, 0, 0, -1, 0, -1, false, 2, false, -1
        } // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel
    };

#elifdef TAMAHANE

#endif

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
                << std::setw(6) << "Sle"
                << std::setw(6) << "DET"
                << std::setw(6) << "POT"
                << std::setw(6) << "BOT"
                << std::setw(6) << "Sct" << "\n";
        ss << std::string(153, '-') << "\n"; // 区切り線を出力
    }

    std::string dumpTable(BattleResult &result, int32_t gene[350], int PastTurns);

    std::string dumpTable(BattleResult &result, int32_t gene[350], int PastTurns) {
        std::stringstream ss6;
        printHeader(ss6);
        int currentTurn = -1;
        int eDamage[2] = {-1, -1}, aDamage = -1;
        bool initiative_tmp = false;
        std::string eAction[2], aAction, sp, tmpState, ATKTurn1, DEFTurn1, magicMirrorTurn1, specialChargeTurn1, amp1,
                ahp2,
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
                    if (!initiative && (action == BattleEmulator::TURN_SKIPPED || action == BattleEmulator::PARALYSIS ||
                                        action == BattleEmulator::SLEEPING)) {
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
                    << std::setw(6) << ((aAction == "Sleeping") ? "yes" : "")
                    << std::setw(6) << DEFTurn1
                    << std::setw(6) << poisonTurn1
                    << std::setw(6) << SpeedTurn1
                    << std::setw(6) << specialChargeTurn1
                    << std::setw(11) << "" << "\n";
        }

        return ss6.str();
    }

    void showHeader() {
#ifdef BUILD_DATE
        const auto buildDate = BUILD_DATE;
#else
        const std::string buildDate = "Unknown";
#endif

#ifdef BUILD_TIME
        const auto buildTime = BUILD_TIME;
#else
        const std::string buildTime = "Unknown";
#endif

        auto compiler = "Unknown";
#if defined(MINGW_BUILD)
        compiler = "mingw";
#elif defined(MSVC_BUILD)
        compiler = "msBuild";
#endif

#if defined(MULTITHREADING)
        std::string multiThreading = ", multithreading is supported, -j " + std::to_string(THREAD_COUNT);
#elif defined(NO_MULTITHREADING)
        std::string multiThreading = ", multithreading is disabled";
#endif
#if defined(OPTIMIZATION_O3_ENABLED)
        std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (Optimized for O3), Build date: " <<
                buildDate
                << ", " <<
                buildTime << " UTC/GMT, Compiler: " << compiler << multiThreading << std::endl;
#elif defined(OPTIMIZATION_O2_ENABLED)
        std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (Optimized for O2), Build date: " << buildDate << ", " << buildTime  << " UTC/GMT, Compiler: " << compiler << multiThreading << std::endl;
#elif defined(NO_OPTIMIZATION)
        std::cout << "dq9 Ragin' Contagion battle emulator " << version << " (No optimization), Build date: " << buildDate << ", " << buildTime   << " UTC/GMT, Compiler: " << compiler << multiThreading << std::endl;
#else
        std::cout << "dq9 Corvus battle emulator" << version << " (Unknown build configuration), Build date: " << buildDate << ", " << buildTime   << " UTC, Compiler: " << compiler << std::endl;
        << ", " << buildTime << std::endl;
#endif
    }

    void help(const char *program_name) {
        std::cout << "Usage: " << program_name << " h m s [actions...]" << std::endl;
        std::cout << "tables" << std::endl;
        std::cout << BattleEmulator::getActionName(BattleEmulator::KASAP) << R"(:   "r" or "k")" << std::endl;
        std::cout << BattleEmulator::getActionName(BattleEmulator::SPECIAL_MEDICINE) << R"(:   "h")" << std::endl;
        std::cout << BattleEmulator::getActionName(BattleEmulator::DECELERATLE) << R"(: "b" or "d")" << std::endl;
        std::cout << BattleEmulator::getActionName(BattleEmulator::SWEET_BREATH) << R"(:      "a" or "s")" << std::endl;
        std::cout << "WARNING: Please input 0 damage attacks (such as shield guard) correctly" << std::endl;
        std::cout << "example: " << program_name << " 2 2 26 29 9 32 9 9 36 9 b" << std::endl;
        std::cout << "example: " << program_name << " 0 2 26 26 r 21 32 r b b 22 35 b 23 36 0 22 h" << std::endl;
        std::cerr << "error: Not enough argc!!" << std::endl;
    }

    NOINLINE bool ProcessInputBuilder(const int argc, char *argv[]) {
        // 4番目以降の引数を `push()` に入れる
        for (int i = 4; i < argc; ++i) {
            if (isMatchStrWithTrim(argv[i], "h")) {
                builder.push(-5);
                continue;
            }
            if (isMatchStrWithTrim(argv[i], "a") || isMatchStrWithTrim(argv[i], "s")) {
                builder.push(-4);
                continue;
            }
            if (isMatchStrWithTrim(argv[i], "b") || isMatchStrWithTrim(argv[i], "d")) {
                builder.push(-2);
                continue;
            }
            if (isMatchStrWithTrim(argv[i], "r") || isMatchStrWithTrim(argv[i], "k")) {
                builder.push(-3);
                continue;
            }
            int damage = toint(argv[i]);
            if (damage >= 0) {
                builder.push(damage);
            } else {
                std::cerr << "Invalid damage value at argv[" << i << "]" << std::endl;
                return false;
            }
        }
        return true;
    }

    NOINLINE int ProgramMain(int hours, int minutes, int seconds) {
        // 構造体の組み合わせを作成
        try {
            auto results = builder.makeStructure();
            for (const auto &result: results) {
                result.print(); // 結果の出力

                // スタックを配列に変換
                int aActions[350] = {0};
                int damages[350] = {0};

                // Aactions をスタックにコピー
                for (int i = 0; i < result.AactionsCounter; ++i) {
                    aActions[i] = result.Aactions[i];
                }
                aActions[result.AactionsCounter] = -1;

                // AII_damage をスタックにコピー
                for (int i = 0; i < result.AII_damageCounter; ++i) {
                    damages[i] = result.AII_damage[i];
                }
                damages[result.AII_damageCounter] = -1;


                FoundSeed = 0;
                foundSeeds = 0;
                auto seed = BruteForceRequest(BasePlayers, hours, minutes, seconds, result.AactionsCounter, aActions,
                                              damages);
                if (foundSeeds == 1) {
                    SearchRequest(BasePlayers, seed, aActions, THREAD_COUNT);
                }
            }
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
        return 0;
    }

    void dumpTableMain(BattleResult &result1, Genome &genome, uint64_t seed, int turns) {
        std::cout << dumpTable(result1, genome.actions, turns) << std::endl;

        std::cout << "0x" << std::hex << seed << std::dec << ": ";

        for (auto i = 0; i < 100; ++i) {
            if (genome.actions[i] == 0 || genome.actions[i] == -1) {
                break;
            }
            std::cout << genome.actions[i] << ", ";
        }
        std::cout << std::endl;
    }

    void PerformanceDebug(const char *name, int turnProcessed, double elapsed_time1, uint64_t seeds) {
        // 正しい計算：1秒あたりの探索回数 (万回/秒)
        double performance = (static_cast<double>(turnProcessed) * 100.0) /
                             static_cast<double>(elapsed_time1);
        performanceLogger << name << ": Turn Consumed: " << turnProcessed << " (" << (
            static_cast<double>(turnProcessed) / 10000) << " mann), ";

        if (seeds != 0) {
            performanceLogger << "Seed Processd: " << (seeds) << "  (" << (
                static_cast<double>(seeds) / 10000) << " mann), ";
        }
        performanceLogger << "elapsed time: " << double(elapsed_time1) / 1000 << " ms, " <<
                "Performance: " << std::fixed << std::setprecision(2) << performance << " mann turns/s" << std::endl;
    }

    bool EasterEgg(int argc, char *argv[]) {
        if (argc > 1 && isMatchStrWithTrim(argv[1], "info2025325")) {
            std::printf("%s\n", repoURL);
            std::printf("%s\n", explanation1);
            std::printf("%s\n", explanation2);
            std::printf("%s\n", explanation3);
            std::printf("%s\n", explanation4);
            return true;
        }
        return false;
    }

#ifdef MULTITHREADING
    void SearchRequest(const Player copiedPlayers[2], uint64_t seed, const int aActions[350], int numThreads) {
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

        auto [turnProcessed,genome] =
                ActionOptimizer::RunAlgorithmAsync(copiedPlayers, seed, turns, 1500, gene, numThreads);

        std::optional<BattleResult> result1;
        result1 = BattleResult();
        Player players[2] = {copiedPlayers[0], copiedPlayers[1]};

        auto *position = new int(1);
        auto *nowState = new uint64_t(0);

        BattleEmulator::Main(position, 100, genome.actions, players, result1, seed, nullptr, nullptr, -1,
                             nowState);

        delete position;
        delete nowState;

#ifdef MINGW_BUILD
        std::cout << turns << std::endl;
        dumpTableMain(result1.value(), genome, seed, 0);
#else
        dumpTableMain(result1.value(), genome, seed, turns - 1);
#endif

#ifdef DEBUG
        auto t3 = std::chrono::high_resolution_clock::now();
        auto elapsed_time1 =
                std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0).count();
        PerformanceDebug("Searcher multi", turnProcessed, static_cast<double>(elapsed_time1), 0);
#endif
    }
#elif NO_MULTITHREADING

    void SearchRequest(const Player copiedPlayers[2], uint64_t seed, const int aActions[350], int numThreads = 1) {
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

        std::optional<BattleResult> result1;
        result1 = BattleResult();
        Player players[2];

        auto *position = new int(1);
        auto *nowState = new uint64_t(0);

        for (int i = 0; i < 1000; ++i) {
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
#ifdef MINGW_BUILD
        std::cout << turns << std::endl;
        dumpTableMain(bestResult, bestGenome, seed, 0);
#else
        dumpTableMain(bestResult, bestGenome, seed, turns - 1);
#endif

#ifdef DEBUG
        auto turnProcessed = BattleEmulator::getTurnProcessed();
        auto t3 = std::chrono::high_resolution_clock::now();
        auto elapsed_time1 =
                std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0).count();
        PerformanceDebug("Searcher single", turnProcessed, static_cast<double>(elapsed_time1), 0);
#endif
    }

#endif

    void BruteForceMainLoop(const Player copiedPlayers[2], uint64_t start, uint64_t end, int turns, int gene[350],
                            int damages[350]) {
        int *position = new int(1);
        auto *nowState = new uint64_t(0);
        int maxElement = 350;
        for (uint64_t seed = start; seed < end; ++seed) {
            lcg::init(seed);
            (*nowState) = 0;
            (*position) = 1;
            Player players[2] = {copiedPlayers[0], copiedPlayers[1]};


            bool resultBool = BattleEmulator::Main(position, 20, gene, players,
                                                   (std::optional<BattleResult> &) std::nullopt, seed, nullptr, damages,
                                                   maxElement,
                                                   nowState);
            if (resultBool) {
                std::cout << seed << std::endl;
                FoundSeed = seed;
                foundSeeds++;
            }
        }
        delete position;
        delete nowState;
    }

    // ブルートフォースリクエスト関数
    [[nodiscard]] uint64_t BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds,
                                             int turns,
                                             int aActions[350], int damages[350]) {
#ifdef DEBUG
        auto t0 = std::chrono::high_resolution_clock::now();
#endif

        std::cout << "BruteForceRequest executed with time " << hours << ":" << minutes << ":" << seconds << std::endl;
        // std::cout << "\naActions: ";
        // for (int i = 0; i < 350 && aActions[i] != -1; ++i) std::cout << aActions[i] << " ";
        // std::cout << "\ndamages: ";
        // for (int i = 0; i < 350 && damages[i] != -1; ++i) std::cout << damages[i] << " ";
        // std::cout << std::endl;
        BattleEmulator::ResetTurnProcessed();

        foundSeeds = 0;
        FoundSeed = 0;

        int totalSeconds = hours * 3600 + minutes * 60 + seconds;
        totalSeconds = totalSeconds - 15;
        auto time1 = static_cast<uint64_t>(floor((totalSeconds - 2.5) * (1 / 0.12515)));
        time1 = time1 << 16;

        auto time2 = static_cast<uint64_t>(floor((totalSeconds + 2.5) * (1 / 0.125155)));
        time2 = time2 << 16;
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

        BruteForceMainLoop(copiedPlayers, time1, time2, turns, gene, damages);

        std::cout << std::endl << "found: " << foundSeeds << std::endl;

        if (foundSeeds == 1) {
#ifdef DEBUG
            auto turnProcessed = BattleEmulator::getTurnProcessed();
            BattleEmulator::ResetTurnProcessed();
            auto t3 = std::chrono::high_resolution_clock::now();
            auto elapsed_time1 =
                    std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0).count();
            PerformanceDebug("BruteForcer", turnProcessed, static_cast<int>(elapsed_time1), time2 - time1);

#endif

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
    NOINLINE std::string ltrim(const std::string &s) {
        size_t start = s.find_first_not_of(" \t\n\r\f\v");
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    // 右側の空白をトリム
    NOINLINE std::string rtrim(const std::string &s) {
        size_t end = s.find_last_not_of(" \t\n\r\f\v");
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }


    // char* を受け取るバージョン（std::stringに変換せず処理）
    NOINLINE std::string trim(const char *s) {
        if (s == nullptr) return "";
        std::string str(s);
        return trim(str);
    }

    // 両側の空白をトリム
    NOINLINE std::string trim(const std::string &s) {
        return rtrim(ltrim(s));
    }

    NOINLINE bool isMatchStrWithTrim(const char *s1, const char *s2) {
        return trim(s1) == trim(s2);
    }
}

int main(int argc, char *argv[]) {
    showHeader();
#ifdef DEBUG
    auto t0 = std::chrono::high_resolution_clock::now();
#endif


    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    //std::cin.tie(0)->sync_with_stdio(0);


#ifdef DEBUG2
    //time1 = 0x199114b2;
    //time1 = 0x226d97a6;
    //time1 = 0x1c2a9bda;
    //time1 = 0x1aa6c05d;
    //3838815720
    //3839393442

    /*
        *3836431220
        3838263295
        3838361070
        3838815720
        3839393442
        3840264243
        */

    uint64_t time1 = 0x05002845;

    int dummy[100];
    lcg::init(time1, false);
    int *position1 = new int(1);

    //0x22f09d67: 25, 25, 25, 57, 57, 25, 57, 54, 56, 25, 25, 25, 25,
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
    int32_t gene1[350] = {0};
    //0x22e2dbaf:

    //int32_t gene1[350] = {25, 25, 50, 27, 54, 57, 56, 25, 54, 57, 57,  BattleEmulator::ATTACK_ALLY};
    //gene1[19-1] = BattleEmulator::DEFENCE;
    int counter = 0;
    //
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::SPECIAL_MEDICINE;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::MIRACLE_SLASH;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::SPECIAL_MEDICINE;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::SPECIAL_MEDICINE;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    gene1[counter++] = BattleEmulator::SPECIAL_MEDICINE;
    // gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    // gene1[counter++] = BattleEmulator::ATTACK_ALLY;
    // gene1[counter++] = BattleEmulator::CRACKLE;
    // gene1[counter++] = BattleEmulator::CRACKLE;
    // gene1[counter++] = BattleEmulator::CRACKLE;
    // gene1[counter++] = BattleEmulator::CRACKLE;
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
    std::memcpy(players1, BasePlayers, sizeof(players1));
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
    uint64_t seed = 0x22d7e800;

    int actions[350] = {BattleEmulator::ATTACK_ALLY, -1,};
    SearchRequest(BasePlayers, seed, actions, THREAD_COUNT);

    std::cout << performanceLogger.rdbuf() << std::endl;

    return 0;
#endif

    if (EasterEgg(argc, argv)) {
        return 1;
    }

    if (argc < 5) {
        help(argv[0]);
        return 1;
    }

    // 戦闘発生時間の取得
    const int hours = toint(argv[1]);
    const int minutes = toint(argv[2]);
    const int seconds = toint(argv[3]);

    if (hours < 0 || minutes < 0 || seconds < 0) {
        std::cerr << "Invalid time parameters" << std::endl;
        return 1;
    }


    ProcessInputBuilder(argc, argv);
    auto exitCode = ProgramMain(hours, minutes, seconds);
    std::cout << performanceLogger.rdbuf();
#ifdef DEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
#endif

    return exitCode;
}
