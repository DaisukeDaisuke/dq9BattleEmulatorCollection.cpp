#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "lcg.h"
#include "BattleEmulator.h"
#include "AnalyzeData.h"
#include "debug.h"

#ifdef DEBUG

#include <chrono>

#endif

int toint(char *string);

//void processResult(const Player *copiedPlayers, const uint64_t seed, std::string input);

std::string ltrim(const std::string &s);

std::string rtrim(const std::string &s);

std::string trim(const std::string &s);

void BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds, int turns, int eActions[500],
                       int aActions[500], int damages[500]);

void mainLoop(const Player copiedPlayers[2]);

using namespace std;

int CandidateID = 0;
int foundSeeds = 0;
std::vector<AnalyzeData> analyzeDataMap;


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
       << std::setw(6) << "ATT"
       << std::setw(6) << "DET"
       << std::setw(6) << "MMT"
       << std::setw(6) << "Tab"
       << std::setw(6) << "Sct" << "\n";
    ss << std::string(140, '-') << "\n";  // 区切り線を出力
}

std::string dumpTable(BattleResult &result, int32_t gene[500],  int PastTurns);

std::string dumpTable(BattleResult &result, int32_t gene[500], int PastTurns) {
    stringstream ss6;
    printHeader(ss6);
    int currentTurn = -1;
    int eDamage[2] = {-1, -1}, aDamage = -1;
    bool initiative_tmp = false;
    std::string eAction[2], aAction, sp, tmpState, ATKTurn1, DEFTurn1, magicMirrorTurn1, specialChargeTurn1, amp1, ahp2, ehp2, amp2;
    auto counter = 0;
    // データのループ
    for (int i = 0; i < result.position; ++i) {
        auto action = result.actions[i];
        auto damage = result.damages[i];
        auto ATKTurn = result.AtkBuffTurns[i];
        auto DEFTurn = result.BuffTurnss[i];
        auto magicMirrorTurn = result.MagicMirrorTurns[i];
        auto turn = result.turns[i];
        auto initiative = result.initiative[i];
        auto ehp1 = result.ehp[i];
        auto ahp1 = result.ahp[i];
        auto isEnemy = result.isEnemy[i];
        auto state = result.state[i] & 0xf;
        auto specialChargeTurn = result.scTurn[i];
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
        if (special != 0&&special != -1) {
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
                            << std::setw(6) << ((aAction == "Sleeping") ? "yes" : "")
                            << std::setw(6) << ATKTurn1
                            << std::setw(6) << DEFTurn1
                            << std::setw(6) << magicMirrorTurn1
                            << std::setw(6) << tmpState
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
            if (ATKTurn >= 0) {
                ATKTurn1 = std::to_string(ATKTurn);
            }
            if (DEFTurn >= 0) {
                DEFTurn1 = std::to_string(DEFTurn);
            }
            if (magicMirrorTurn >= 0) {
                magicMirrorTurn1 = std::to_string(magicMirrorTurn);
            }
            if (specialChargeTurn > 0) {
                specialChargeTurn1 = std::to_string(specialChargeTurn);
            }

            amp1 = std::to_string(amp);

            initiative_tmp = initiative;
            sp = specialAction;
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
                << std::setw(6) << ATKTurn1
                << std::setw(6) << DEFTurn1
                << std::setw(6) << magicMirrorTurn1
                << std::setw(6) << tmpState
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

//int main(int argc, char *argv[]) {
int main() {

    std::cout << "dq9 Corvus battle emulator v1.0.0" << std::endl << "Waiting for input[q/u/b/s]: " << std::endl;

#ifdef DEBUG2
    std::cout << sizeof(int) << std::endl;
    std::cout << sizeof(long) << std::endl;
    std::cout << sizeof(long long) << std::endl;
    std::cout << sizeof(uint64_t) << std::endl;
#endif

#ifdef DEBUG
    auto t0 = std::chrono::high_resolution_clock::now();
#endif

    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    //std::cin.tie(0)->sync_with_stdio(0);

    const Player copiedPlayers[2] = {
            // プレイヤー1
            {309,  309.0,  312, 312, 298, 298, 193, 234, 165,   // 最初のメンバー
                    165, false, false, 0, false, false, 0, -1,              // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
                    8, 1.0, false, -1, 0, -1,                                            // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
                    false, -1, 0, -1, 0, false, 1, 1, 1},                                      // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel

            // プレイヤー2
            {4800, 4800.0, 248, 248, 278, 278, 157, 0,   255,    // 最初のメンバー
                    255, false, false, 0, false, false, 0, -1,             // specialCharge, dirtySpecialCharge, specialChargeTurn, inactive, paralysis, paralysisLevel, paralysisTurns
                    8, 1.0, false, -1, 0, -1,                         // SpecialMedicineCount, defence, sleeping, sleepingTurn, BuffLevel, BuffTurns
                    false, -1, 0, -1, 0, false, 0, 0, 0}                             // hasMagicMirror, MagicMirrorTurn, AtkBuffLevel, AtkBuffTurn, TensionLevel
    };

#ifdef DEBUG2
    //time1 = 0x199114b2;
    //time1 = 0x226d97a6;
    //time1 = 0x1c2a9bda;
    //time1 = 0x1aa6c05d;
    uint64_t time1 = 1650655705;

    int dummy[100];
    lcg::init(time1);
    int *position1 = new int(1);
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

    auto *NowState = new uint64_t(0);//エミュレーターの内部ステートを表すint

    Player players1[2];
    int32_t gene1[500] = {0};
    //gene1[19-1] = BattleEmulator::DEFENCE;
    int counter = 0;

    gene1[counter++] = BattleEmulator::BUFF;
    gene1[counter++] = BattleEmulator::MAGIC_MIRROR;
    gene1[counter++] = BattleEmulator::BUFF;
    gene1[counter++] = BattleEmulator::DOUBLE_UP;
    gene1[counter++] = BattleEmulator::MAGIC_MIRROR;
    gene1[counter++] = BattleEmulator::BUFF;
    gene1[counter++] = BattleEmulator::BUFF;
    gene1[counter++] = BattleEmulator::DEFENDING_CHAMPION;
    gene1[counter++] = BattleEmulator::FULLHEAL;

    //for (int i = 0; i < 10; ++i) {
        (*NowState) = 0;
        (*position1) = 1;
        std::optional<BattleResult> dummy1;
        dummy1 = BattleResult();
        std::memcpy(players1, copiedPlayers, sizeof(players1));
        BattleEmulator::Main(position1, 43, gene1, players1, dummy1, time1, dummy, dummy, -1, NowState);

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
    time1 = 57086815;

    lcg::init(time1, 5000);
    processResult(copiedPlayers, time1, gene, 0, "11 18 15 9 18 9 10 16 h", -1);
    lcg::release();
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


// ブルートフォースリクエスト関数
void BruteForceRequest(const Player copiedPlayers[2], int hours, int minutes, int seconds,int turns,  int eActions[500],
                       int aActions[500], int damages[500]) {
    std::cout << "BruteForceRequest executed with time " << hours << ":" << minutes << ":" << seconds << std::endl;
    std::cout << "eActions: ";
    for (int i = 0; i < 500 && eActions[i] != -1; ++i) std::cout << eActions[i] << " ";
    std::cout << "\naActions: ";
    for (int i = 0; i < 500 && aActions[i] != -1; ++i) std::cout << aActions[i] << " ";
    std::cout << "\ndamages: ";
    for (int i = 0; i < 500 && damages[i] != -1; ++i) std::cout << damages[i] << " ";
    std::cout << std::endl;

    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    totalSeconds = totalSeconds - 15;
    //std::cout << totalSeconds << std::endl;
    auto time1 = static_cast<uint64_t>(floor((totalSeconds - 60) * (1 / 0.12515)));
    time1 = (time1 & 0xffff) << 16;
    std::cout << time1 << std::endl;

    auto time2 = static_cast<uint64_t>(floor((totalSeconds + 60) * (1 / 0.125155)));
    time2 = (time2 & 0xffff) << 16;
    std::cout << time2 << std::endl;
////
    time1 = 0;
    time2 = 4294967296;
//
    int32_t gene[500] = {0};
    for (int i = 0; i < 500; ++i) {
        gene[i] = aActions[i];
        if (aActions[i] == -1){
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
    int maxElement = 500;
    Player players[2];

    if (time1 < time2) {
        for (uint64_t seed = time1; seed < time2; ++seed) {
            if (seed % 1000000000 == 0){
                std::cout << seed << std::endl;
            }
            lcg::init(seed);
            for (int st = BattleEmulator::TYPE_2A; st < BattleEmulator::TYPE_2D; ++st) {
                (*nowState) = st;
                (*position) = 1;
                //std::memcpy(players, copiedPlayers, sizeof(players));
                players[0] = copiedPlayers[0];
                players[1] = copiedPlayers[1];



                bool resultBool = BattleEmulator::Main(position, turns, gene, players,
                                                       (optional<BattleResult> &) std::nullopt, seed, eActions, damages,
                                                       maxElement,
                                                       nowState);
                if (resultBool) {
                    std::cout << seed << ", " << st << std::endl;
                }
            }
        }
    } else {
        bool reset = false;
        // オーバーフローが発生している場合
        for (uint64_t seed = time1; seed < 4294967296; ++seed) {
            for (int st = BattleEmulator::TYPE_2A; st < BattleEmulator::TYPE_2E; ++st) {
                (*nowState) = st;
                (*position) = 1;
                lcg::init(seed);
                // ここに処理を書く
                std::memcpy(players, copiedPlayers, sizeof(players));

                bool resultBool = BattleEmulator::Main(position, turns, gene, players,
                                                       (optional<BattleResult> &) std::nullopt, seed, eActions, damages,
                                                       maxElement,
                                                       nowState);
                if (resultBool) {
                    std::cout << seed << std::endl;
                }
            }
            if (seed == 4294967295) {
                seed = 0; // seed を 0 にリセット
                reset = true;
            }

            if (reset && seed >= time2) {
                break; // time2 に達したらループを終了
            }
        }
    }



//    for (uint64_t seed = time1; seed < time2; ++seed) {
//        (*nowState) = 0;
//        (*position) = 1;
//        lcg::init(seed);
//
//        std::memcpy(players, copiedPlayers, sizeof(players));
//
//        bool resultBool = BattleEmulator::Main(position, 25, std::vector<int32_t>(), players,
//                                               (optional<BattleResult> &) std::nullopt, seed, eActions, damages, maxElement,
//                                               nowState);
//
//        if (resultBool) {
//            processResult(copiedPlayers, seed, "str2");
//            foundSeeds++;
//        }
//    }
    delete position;
    delete nowState;

    std::cout << std::endl << "found: " << foundSeeds << std::endl;
}

// 入力文字列を配列に分割するヘルパー関数
void parseActions(const std::string &str, int actions[500]) {
    std::istringstream iss(str);
    int value, index = 0;
    while (iss >> value && index < 500) {
        actions[index++] = value;
    }
    actions[index++] = -1;
    actions[index++] = -1;
    actions[index++] = -1;
}


// メインループ
void mainLoop(const Player copiedPlayers[2]) {
    int eActions[500] = {0};
    int aActions[500] = {0};
    int damages[500] = {0};

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        char command = input[0];
        if (command == 'q') {
            std::cout << "Exiting loop." << std::endl;
            break;
        }

        std::istringstream ss(input.substr(2));  // コマンド文字を除外してパース

        int hours, minutes, seconds, turns;
        ss >> hours >> minutes >> seconds >> turns;

        // '/'区切りでeActions, aActions, damagesに分割
        std::string eActionsStr, aActionsStr, damagesStr;
        std::getline(ss, eActionsStr, '-');
        std::getline(ss, aActionsStr, '-');
        std::getline(ss, damagesStr, '-');

        // 各アクション配列に値を代入
        parseActions(eActionsStr, eActions);
        parseActions(aActionsStr, aActions);
        parseActions(damagesStr, damages);

        // コマンドに応じた処理
        if (command == 'b') {
            BruteForceRequest(copiedPlayers, hours, minutes, seconds,turns, eActions, aActions, damages);
        } else if (command == 'u') {
            std::cout << "Updating state..." << std::endl;
            // 状態更新の処理を実装
        } else {
            std::cerr << "Unknown command." << std::endl;
        }
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
