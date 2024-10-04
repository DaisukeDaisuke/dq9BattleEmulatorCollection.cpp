#include <iostream>
#include <ctime>        // time
#include <cstdlib>      // srand,lcg
#include <cstdio>
#include <cstring>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iomanip>
#include "lcg.h"
#include "BattleEmulator.h"
#include "AnalyzeData.h"
#include "debug.h"

int toint(char *string);

void processResult(const Player *copiedPlayers, const uint64_t seed, std::vector<int32_t> gene, int depth,
                   std::string input);

std::string ltrim(const std::string &s);

std::string rtrim(const std::string &s);

std::string trim(const std::string &s);

using namespace std;

int CandidateID = 0;
int foundSeeds = 0;
std::vector<AnalyzeData> analyzeDataMap;


void printHeader(std::stringstream &ss);

// ヘッダーを出力する関数
void printHeader(std::stringstream &ss) {
    ss << std::left << std::setw(6) << "turn"
        << std::setw(16) << "sp"
        << std::setw(16) << "aAct"
        << std::setw(16) << "eAct"
        << std::setw(6) << "aD"
        << std::setw(6) << "eD"
        << std::setw(6) << "ahp"
        << std::setw(6) << "ehp"

        << std::setw(6) << "ini"
        << std::setw(6) << "Para"
        << std::setw(6) << "Ina" << "\n";
    ss << std::string(95, '-') << "\n";  // 区切り線を出力
}

std::string dumpTable(BattleResult &result, std::vector<int32_t> gene, int PastTurns);

std::string dumpTable(BattleResult &result, std::vector<int32_t> gene, int PastTurns) {
    stringstream ss6;
    printHeader(ss6);
    int currentTurn = -1;
    int eDamage = -1, aDamage = -1;
    bool isP1, isI1, initiative_tmp = false;
    std::string eAction, aAction, sp;

    // データのループ
    for (int i = 0; i < result.position; ++i) {
        auto action = result.actions[i];
        auto damage = result.damages[i];
        auto isP = result.isParalysis[i];
        auto isI = result.isInactive[i];
        auto turn = result.turns[i];
        auto initiative = result.initiative[i];
        auto ehp1 = result.ehp[i];
        auto ahp1 = result.ahp[i];
        auto isEnemy = result.isEnemy[i];

        auto special = gene[turn];
        std::string specialAction;
        if (special != 0) {
            specialAction = BattleEmulator::getActionName(special & 0x3ff);
        }

        // ターンが変わったら、前のターンのデータを出力
        if (turn != currentTurn) {
            if (currentTurn != -1) {
                // 前のターンの出力
                if (turn > PastTurns) {
                    ss6
                            << std::left << std::setw(6) << (currentTurn + 1)
                            << std::setw(16) << sp
                            << std::setw(16) << aAction
                            << std::setw(16) << eAction
                            << std::setw(6) << aDamage
                            << std::setw(6) << eDamage
                            << std::setw(6) << ahp1
                            << std::setw(6) << ehp1
                            << std::setw(6) << (initiative_tmp ? "yes" : "")
                            << std::setw(6) << ((aAction == "Paralysis" || aAction == "Cure Paralysis") ? "yes" : "")
                            << std::setw(6) << ((aAction == "Inactive") ? "yes" : "")
                            << std::setw(11) << "" << "\n";
                }
            }
            // ターンの初期化
            currentTurn = turn;
            eAction = "";
            aAction = "";
            eDamage = 0;
            aDamage = 0;
            sp = "";
            initiative_tmp = false;
        }

        // 敵か味方の行動を適切な変数に格納
        if (isEnemy) {
            eAction = BattleEmulator::getActionName(action);
            eDamage = damage;
            isI1 = isI1 || isI;
        } else {
            aAction = BattleEmulator::getActionName(action);
            aDamage = damage;
            isP1 = isP;
            isI1 = isI;
            initiative_tmp = initiative;
            sp = specialAction;
        }
    }

    // 最後のターンのデータを出力
    if (currentTurn != -1) {
        ss6
                << std::left << std::setw(6) << (currentTurn + 1)
                << std::setw(16) << sp
                << std::setw(16) << aAction
                << std::setw(16) << eAction
                << std::setw(6) << aDamage
                << std::setw(6) << eDamage
                << std::setw(6) << result.ahp[result.position - 1]
                << std::setw(6) << result.ehp[result.position - 1]
                << std::setw(6) << (initiative_tmp ? "yes" : "")
                << std::setw(6) << ((aAction == "Paralysis" || aAction == "Cure Paralysis") ? "yes" : "")
                << std::setw(6) << ((aAction == "Inactive") ? "yes" : "")
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
        auto isP = result.isParalysis[i];
        auto isI = result.isInactive[i];
        if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
            ss << "h ";
            counter++;
        } else if (damage != 0) {
            counter++;
            if (isP) {
                ss << damage << "-m" << " ";
            } else if (isI) {
                ss << damage << "-i" << " ";
            } else {
                ss << damage << " ";
            }
        }
        if (counter == 10) {
            ss << std::endl;
            counter = 0;
        }
    }
    auto turn = result.turn + 1;
    if (data.getWinStatus()) {
        ss << "W " << turn;
    }else{
        ss << "L " << turn;
    }
    return ss.str();
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        std::cerr << "argc!!" << std::endl;
        return 1;
    }
    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    std::cin.tie(0)->sync_with_stdio(0);

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<int32_t> gene = std::vector<int32_t>(100, 0);

    Player players[2];
    int hps[2] = {70, 456}; //hp変更する際はproportionTable3を絶対変更すること。どこにあるか分からない場合はclionのctrl+shift+fを使うべし。
    int defs[2] = {69, 58};
    int atks[2] = {62 + 2, 56};
    int speeds[2] = {44, 54};
    int mps[2] = {24, 255};
    for (int i = 0; i < 2; ++i) {
        players[i].playerId = i;
        players[i].hp = hps[i];
        players[i].maxHp = hps[i];
        players[i].atk = atks[i];
        players[i].def = defs[i];
        players[i].speed = speeds[i];
        players[i].mp = mps[i];
    }
    int hours = toint(argv[1]);
    int minutes = toint(argv[2]);
    int seconds = toint(argv[3]);

    Player copiedPlayers[2];
    for (int i = 0; i < 2; ++i) {
        copiedPlayers[i] = players[i];
    }

    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    totalSeconds = totalSeconds - 15;
    //std::cout << totalSeconds << std::endl;
    auto time1 = static_cast<uint64_t>(floor((totalSeconds - 1) * (1 / 0.12515)));
    time1 = (time1 & 0xffff) << 16;
    //std::cout << time1 << std::endl;

    auto time2 = static_cast<uint64_t>(floor((totalSeconds + 1) * (1 / 0.125155)));
    time2 = (time2 & 0xffff) << 16;

//    time1 = 166779029;
//    time2 = 166779031;

#ifdef DEBUG2
    time1 = 52736593;
    time2 = 2501309586;

    lcg::init(time1, 5000);
    int *position = new int(1);
    for (int j = 0; j < 2; ++j) {
        players[j] = copiedPlayers[j];
    }
    vector<int32_t> gene1(gene);
    gene1[18-1] = BattleEmulator::DEFENCE;
    BattleResult result;
    BattleEmulator::Main(position, 100, gene1, players, result, time1);
    delete position;
    lcg::release();

    std::stringstream ss1;
    ss1 << time1 << " ";
    int counter = 0;

    std::cout << dumpTable(result, gene1, -1) << std::endl;
    return 0;
#endif

#ifdef DEBUG3
    time1 = 166779030;

    lcg::init(time1, 5000);
    processResult(copiedPlayers, time1, gene, 0, "15 9 10 18 9 17 13 15 h");
    lcg::release();
    return 0;
#endif

    //std::cout << time2  << std::endl;
    // Now you can use the precalculated values as needed
    //int test = 0;
    //std::vector<std::stringstream> streams(100);
    //for (uint64_t seed = 0; seed < 100000; ++seed) {
    //std::string str2 = "10 18 19 10 h 9 17 8 18 11 9 h 18";
    //std::string str2 = "19 9 9 17 h 13 15 10 10 11 11 h 15";
    //std::string str2 = argv[5];//"18 9 15 18 15 h 19 9 14";

    std::stringstream ss;

    // argv[5]以降をstringstreamに入れる
    for (int i = 4; i < argc; ++i) {
        ss << argv[i] << " ";
    }

    std::string str2 = ss.str();

    //str2.erase(std::remove(str2.begin(), str2.end(), '"'), str2.end());

//    time1 = 0x98087FD0;
//    time2 = 0x98087FD0+1;
    for (uint64_t seed = time1; seed < time2; ++seed) {
        if (seed % 10000 == 0) {
            //std::cout << seed << std::endl;
        }
        lcg::init(seed, 5000);
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }
        BattleResult result;
        BattleEmulator::Main(position, 25, gene, players, result, seed);
        std::stringstream ss;
        ss << seed << " ";
        for (int i = 0; i < result.position; ++i) {
            auto action = result.actions[i];
            auto damage = result.damages[i];
            if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
                ss << "h ";
            } else if (damage != 0) {
                ss << damage << " ";
            }
        }
        if (players[0].hp <= 0) {
            ss << "L ";
        }
        if (players[1].hp <= 0) {
            ss << "W ";
        }
        std::string resultStr = ss.str();

        //if (resultStr.find(str2) != std::string::npos) {
        if (resultStr.rfind(std::to_string(seed) + " " + str2, 0) == 0) {
            //std::cout << seed << std::endl;
            //std::cout << resultStr << std::endl;
            processResult(copiedPlayers, seed, gene, 0, str2);
            foundSeeds++;
        }
        lcg::release();
        delete position;
    }

    std::cout << std::endl << "found: " << foundSeeds << std::endl;

    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    //std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
    return 0;
}


void processResult(const Player *copiedPlayers, const uint64_t seed, std::vector<int32_t> gene1, int depth,
                   std::string input) {
    BattleResult result2;
    vector<AnalyzeData> candidate = vector<AnalyzeData>();
    int PreviousTurn = 0;
    int lastParalysis = -1;
    int lastCounter = -1;
    int paralysisTurns = 0;
    bool first = false;
    std::map<int32_t, int> paralysis_map;
    int lastInputTurn = -1;
    auto respite = -1;

    std::cout << "============" << seed  << "============" << std::endl;

    {
        std::stringstream ss1;
        std::stringstream ss5;
        std::stringstream ss6;
        ss1 << seed << " " << std::endl;
        Player players[2];
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }

        std::vector<int32_t> gene(gene1);
        BattleEmulator::Main(position, 200, gene, players, result2, seed);
        int counter = 0;
        AnalyzeData analyzeData;
        analyzeData.FromBattleResult(result2);
        analyzeData.setGenome(gene);


        for (int i = 0; i < result2.position; ++i) {
            auto action = result2.actions[i];
            auto damage = result2.damages[i];
            auto isP = result2.isParalysis[i];
            auto isI = result2.isInactive[i];
            auto turn = result2.turns[i];
            if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
                ss1 << "h ";
                ss5 << "h ";
                counter++;
            } else if (damage != 0) {
                if (isP) {
                    ss1 << damage << "-m" << " ";
                    ss5 << damage << " ";
                } else if (isI) {
                    ss1 << damage << "-i" << " ";
                    ss5 << damage << " ";
                } else {
                    ss1 << damage << " ";
                    ss5 << damage << " ";
                }
                counter++;
            }
            std::string trimmedInput = trim(input);
            std::string trimmedSS5 = trim(ss5.str());

            if (!trimmedInput.empty() && trimmedInput == trimmedSS5) {
                lastInputTurn = turn;
            }
            if (counter == 10) {
                ss1 << std::endl;
                counter = 0;
            }
        }
        auto table = dumpTable(result2, gene, -1);


        auto turn = result2.turn + 1;
        respite = turn - lastInputTurn;
        if (players[0].hp <= 0) {
            ss1 << "L " << turn;
            analyzeData.setBattleTrace("L " + std::to_string(turn) + "\n");
            std::cout << table << std::endl << "lost" << std::endl;
            analyzeData.setWinStatus(false);
        }
        if (players[1].hp <= 0) {
            ss1 << "W " << turn;
            analyzeData.setBattleTrace("W " + std::to_string(turn) + "\n");
            std::cout << table << std::endl << "win!" << std::endl;
            analyzeData.setWinStatus(true);
        }
        std::cout << ss1.str() << std::endl;
        candidate.push_back(analyzeData);
        delete position;
    }

    for (int j = result2.position - 1; j >= 0; --j) {
        if (!result2.isEnemy[j]) {
            // ss1 << lastParalysis << ": " << paralysisTurns << ", ";
            int action = result2.actions[j];
            auto paralysis = result2.isParalysis[j];
            auto inactive = result2.isInactive[j];
            auto player0_has_initiative = result2.initiative[j];
            auto turn = result2.turns[j];
            auto ehp = result2.ehp[j];
            auto ahp = result2.ahp[j];
            if (respite >= 8) {
                if (turn <= (lastInputTurn+3)) {
                    continue;
                }
            }else{
                if (turn <= lastInputTurn) {
                    continue;
                }
            }
            //ss1 << action << ": " << enemy.ehp << ": " << result.turn[j] << ", ";
            if (action == BattleEmulator::PARALYSIS ||
                (action == BattleEmulator::INACTIVE_ALLY) ||
                action == BattleEmulator::CURE_PARALYSIS || inactive || paralysis) {
                continue;
            }
            paralysis_map[turn] = (turn << 20) | (ehp << 10) | ahp;
        }
    }
    for (const auto &pair: paralysis_map) {
        std::stringstream ss;
        int ehp = ((pair.second >> 10) & 0x3ff);
        int ahp = (pair.second & 0x3ff);
        int turns = ((pair.second >> 20) & 0x3ff);
        //ss << "hp: " << ((pair.second >> 10) & 0x3ff) << ", hp1: " << (pair.second & 0x3ff) << ", turn: " << ((pair.second >> 20) & 0x3ff) << ", defense: ";
        Player players[2];
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }
        BattleResult result1;
        //gene[pair.first] = (ahp << 20) | (ehp << 10) | BattleEmulator::DEFENCE;
        std::vector<int32_t> gene(gene1);
        gene[pair.first] = (ahp << 20) | (ehp << 10) | BattleEmulator::DEFENCE;
        BattleEmulator::Main(position, 200, gene, players, result1, seed);
        AnalyzeData analyzeData;
        analyzeData.FromBattleResult(result1);
        analyzeData.setGenome(gene);
        //std::cout << analyzeData.calculateEfficiency() << std::endl;

        auto turn = result1.turn + 1;
        if (players[0].hp <= 0) {
            ss << "L " << turn << ", seed: " << seed;
            analyzeData.setWinStatus(false);
        }
        if (players[1].hp <= 0) {
            ss << "W " << turn << ", seed: " << seed;
            analyzeData.setWinStatus(true);
        }
        ss << std::endl;
        analyzeData.setBattleTrace(ss.str());
        candidate.push_back(analyzeData);
        delete position;
        //std::cout << ss.str() << endl;
    }

    for (const auto &pair: paralysis_map) {
        std::stringstream ss;
        int ehp = ((pair.second >> 10) & 0x3ff);
        int ahp = (pair.second & 0x3ff);
        int turns = ((pair.second >> 20) & 0x3ff);
        //ss << "hp: " << ((pair.second >> 10) & 0x3ff) << ", hp1: " << (pair.second & 0x3ff) << ", turn: " << ((pair.second >> 20) & 0x3ff) << ", heal: ";
        Player players[2];
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }
        BattleResult result3;
        std::vector<int32_t> gene(gene1);
        gene[pair.first] = (ahp << 20) | (ehp << 10) | BattleEmulator::HEAL;
        BattleEmulator::Main(position, 200, gene, players, result3, seed);
        AnalyzeData analyzeData;
        analyzeData.FromBattleResult(result3);
        analyzeData.setGenome(gene);
        analyzeData.setLastInputTurn(lastInputTurn);

        auto turn = result3.turn + 1;
        if (players[0].hp <= 0) {
            ss << "L " << turn << ", seed: " << seed;
            analyzeData.setWinStatus(false);
        }
        if (players[1].hp <= 0) {
            ss << "W " << turn << ", seed: " << seed;
            analyzeData.setWinStatus(true);
        }
        ss << std::endl;
        analyzeData.setBattleTrace(ss.str());
        candidate.push_back(analyzeData);
        delete position;
        //std::cout << ss.str() << endl;
    }





    // 最高の効率を持つ要素を見つける
    // 候補を効率の高い順にソート
    std::sort(candidate.begin(), candidate.end(),
              [](const AnalyzeData &a, const AnalyzeData &b) {
                  return a.calculateEfficiency() > b.calculateEfficiency(); // 降順でソート
              });

    int lastTurn = -1;
    // ソートされた候補を処理
    int found = 0;
    AnalyzeData lastData;

    for (AnalyzeData &data: candidate) {
        //std::cout << "efficiency: " << data.calculateEfficiency() << std::endl;
        if (data.calculateEfficiency() > 7) {
            if (data.getWinStatus()) {
                std::stringstream ss7;
                found++;
                ss7 << std::endl << std::endl << "=======cid " << CandidateID << "========" << std::endl
                    << data.getBattleTrace();
                ss7 << "actions: " << std::endl;

                auto vec = data.getGenome();
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] != 0) {
                        int action = (vec[i] & 0x3ff);
                        std::string actionName;
                        if (action == BattleEmulator::HEAL) {
                            actionName = "HEAL";
                        } else if (action == BattleEmulator::DEFENCE) {
                            actionName = "DEFENCE";
                        }
                        lastTurn = i + 1;
                        ss7 << "turn: " << lastTurn << ", ehp: " << ((vec[i] >> 10) & 0x3ff) << ", ahp: "
                            << ((vec[i] >> 20) & 0x3ff) << ", action: " << actionName << std::endl;
                    }
                }
                std::cout << ss7.str();
                data.setEvaluationString(ss7.str());
                analyzeDataMap.push_back(data);
                CandidateID++;
                lastData = data;
                break;
            }
        }
        // 他の処理をここに追加...
    }
    if (found != 0) {
        std::cout << dumpTable(*lastData.getBattleResult(), lastData.getGenome(), lastInputTurn) << std::endl << normalDump(lastData)
                  << std::endl;
    }
    std::cout << "============" << seed  << "============" << std::endl;
    return;
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