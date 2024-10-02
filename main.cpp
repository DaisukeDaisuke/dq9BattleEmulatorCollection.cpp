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
#include "lcg.h"
#include "BattleEmulator.h"
#include "AnalyzeData.h"
#include "debug.h"

int toint(char *string);
void processResult(const Player *copiedPlayers,const uint64_t seed,  std::vector<int32_t> gene,int depth);

using namespace std;

int CandidateID = 0;

int main(int argc, char *argv[]) {
    if (argc < 6){
        std::cerr << "argc!!" << std::endl;
        return 1;
    }
    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    std::cin.tie(0)->sync_with_stdio(0);

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<int32_t> gene = std::vector<int32_t>(100, 0);

    Player players[2];
    int hps[2] = {70, 456};
    int defs[2] = {69, 58};
    int atks[2] = {62+2, 56};
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



//        auto time = (static_cast<int>(floor(((48 * 60 + 0) * 0.19)))) << 16;
//        auto time1 = (static_cast<int>(floor(((50 * 60 + 0) * 0.19)))) << 16;
//        auto time = (static_cast<int>(floor(((50 * 60 + 50) * 0.19)))) << 16;
//        auto time1 = (static_cast<int>(floor(((60 * 60 + 50) * 0.19)))) << 16;
    //b 20 17 21 18 21 31 26 22 17
//        auto time = 0x025C5A0D-2;
//        auto time1 = 0x025C5A0D+2;

    //int i = 0x025C5A0D;
    //b z 22 A 27 17 19 28
    //b a 25 16 z 22 16 22 22

//        //b z 22 A 27 22 16 22 9 17 21 21 18
//        for (int i = time; i <time1; ++i) {
//            int *position = new int(1);
//            lcg::init(i, 500);
//            for (int j = 0; j < 2; ++j) {
//                players[j] = copiedPlayers[j];
//            }
//            if(BattleEmulator::Main(position, gene, players)){
//                std::cout << "found: " << i << std::endl;
//            }
//            lcg::release();
//            delete position;
//        }
    // Declare and initialize an array to store precalculated values
    //b a 25 16 z 22 16 22
    //uint64_t seed = 36434887;
    //uint64_t seed = 0x31DEF5AB;
    //uint64_t seed = 0x30ffea3a;//幼女2回目
    //uint64_t seed = 0x45F7ADf0;//幼女3回目、2ターン目必殺チャージ
    //uint64_t seed = 0x30f24ab7;//幼女4回目
    //uint64_t seed = 0x31D66981;
    //uint64_t seed = 0x127578ED;

//    int hours = 1;
//    int minutes = 21;
//    int seconds = 28;
//    int hours = 1;
//    int minutes = 27;
//    int seconds = 58;
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


#ifdef DEBUG2
    time1 = 2327755714;
    time2 = 2501309586;

    lcg::init(time1, 5000);
    int *position = new int(1);
    for (int j = 0; j < 2; ++j) {
        players[j] = copiedPlayers[j];
    }
    vector<int32_t> gene1(gene);
    //gene1[21-1] = BattleEmulator::DEFENCE;
    BattleResult result;
    BattleEmulator::Main(position, 100, gene1, players, result, time1);
    delete position;
    lcg::release();

    std::stringstream ss1;
    ss1 << time1 << " ";
    int counter = 0;
    for (int i = 0; i < result.position; ++i) {
        auto action = result.actions[i];
        auto damage = result.damages[i];
        auto isP = result.isParalysis[i];
        auto isI = result.isInactive[i];
        if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
            ss1 << "h ";
            counter++;
        } else if (damage != 0) {
            if (isP){
                ss1 << damage << "-m" << " ";
            }else if(isI){
                ss1 << damage << "-i" << " ";
            }else {
                ss1 << damage << " ";
            }
            counter++;
        }
        if(counter == 10){
            ss1 << std::endl;
            counter = 0;
        }
    }
    auto turn = result.turn;
    if (players[0].hp <= 0){
        ss1 << "L " << turn;
    }
    if (players[1].hp <= 0){
        ss1 << "W " << turn;
    }
    std::string resultStr = ss1.str();
    std::cout << resultStr << std::endl;
    return 0;
#endif

#ifdef DEBUG3
    time1 = 0x1000;

    lcg::init(time1, 5000);
    processResult(copiedPlayers, time1, gene, 0);
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
        if (players[0].hp <= 0){
            ss << "L ";
        }
        if (players[1].hp <= 0){
            ss << "W ";
        }
        std::string resultStr = ss.str();

        //if (resultStr.find(str2) != std::string::npos) {
        if (resultStr.rfind(std::to_string(seed) + " " + str2, 0) == 0) {
            //std::cout << seed << std::endl;
            //std::cout << resultStr << std::endl;
            processResult(copiedPlayers, seed, gene, 0);
        }
        lcg::release();
        delete position;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    //std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
    //std::cout << (*position) << std::endl;

    //cout << "hp" << endl;
//    for (auto &player: players) {
//        std::cout << player.hp << std::endl;
//    }
    //std::cout << (players[0].specialCharge ? "hissatu: true" : "hissatu: fa") << std::endl;

    //std::cout << "Size of int: " << sizeof(int) * 8 << " bits" << std::endl;

    //delete position;


    return 0;
}


void processResult(const Player *copiedPlayers, const uint64_t seed, std::vector<int32_t> gene1, int depth) {
    BattleResult result2;
    vector<AnalyzeData> candidate = vector<AnalyzeData>();
    int PreviousTurn = 0;
    int lastParalysis = -1;
    int lastCounter = -1;
    int paralysisTurns = 0;
    bool first = false;
    std::map<int32_t, int> paralysis_map;

    {
        std::stringstream ss1;
        ss1 << seed << " " << std::endl;
        Player players[2];
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }

        std::vector<int32_t> gene(gene1);
        BattleEmulator::Main(position, 200, gene, players, result2, seed);
        int counter = 0;
        for (int i = 0; i < result2.position; ++i) {
            auto action = result2.actions[i];
            auto damage = result2.damages[i];
            auto isP = result2.isParalysis[i];
            auto isI = result2.isInactive[i];
            if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
                ss1 << "h ";
                counter++;
            } else if (damage != 0) {
                if (isP){
                    ss1 << damage << "-m" << " ";
                }else if(isI){
                    ss1 << damage << "-i" << " ";
                }else {
                    ss1 << damage << " ";
                }
                counter++;
            }
            if(counter == 10){
                ss1 << std::endl;
                counter = 0;
            }
        }
        auto turn = result2.turn;
        if (players[0].hp <= 0){
            ss1 << "L " << turn;
        }
        if (players[1].hp <= 0){
            ss1 << "W " << turn;
        }
        std::cout << ss1.str() << std::endl;
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
            //ss1 << action << ": " << enemy.ehp << ": " << result.turn[j] << ", ";
            if (action == BattleEmulator::PARALYSIS || (paralysisTurns > 0 && action == BattleEmulator::INACTIVE_ALLY) || action == BattleEmulator::CURE_PARALYSIS || inactive || paralysis) {
                paralysisTurns++;
                lastParalysis = turn;
                lastCounter = j;
                continue;
            }
            if(first&&action != BattleEmulator::INACTIVE_ALLY){
                paralysis_map[turn] = (turn << 20) | (ehp << 10) | ahp;
                first = false;
            }
            if (paralysisTurns > 0&&action != BattleEmulator::INACTIVE_ALLY) {
                first = true;
                paralysis_map[turn] = (turn << 20) | (ehp << 10) | ahp;
            }
            lastParalysis = -1;
            lastCounter = -1;
            paralysisTurns = 0;
        }
    }
    for (const auto& pair : paralysis_map) {
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

        int counter = 0;
        for (int i = 0; i < result1.position; ++i) {
            auto action = result1.actions[i];
            auto damage = result1.damages[i];
            auto isP = result1.isParalysis[i];
            auto isI = result1.isInactive[i];
            if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
                ss << "h ";
                counter++;
            } else if (damage != 0) {
                counter++;
                if (isP){
                    ss << damage << "-m" << " ";
                }else if(isI){
                    ss << damage << "-i" << " ";
                }else {
                    ss << damage << " ";
                }
            }
            if(counter == 10){
                ss << std::endl;
                counter = 0;
            }
        }
        auto turn = result1.turn;
        if (players[0].hp <= 0){
            ss << "L " << turn;
            analyzeData.setWinStatus(false);
        }
        if (players[1].hp <= 0){
            ss << "W " << turn;
            analyzeData.setWinStatus(true);
        }
        ss << std::endl;
        analyzeData.setBattleTrace(ss.str());
        candidate.push_back(analyzeData);
        delete position;
        //std::cout << ss.str() << endl;
    }

    for (const auto& pair : paralysis_map) {
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

        int counter = 0;
        for (int i = 0; i < result3.position; ++i) {
            auto action = result3.actions[i];
            auto damage = result3.damages[i];
            auto isP = result3.isParalysis[i];
            auto isI = result3.isInactive[i];
            if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
                ss << "h ";
                counter++;
            } else if (damage != 0) {
                counter++;
                if (isP){
                    ss << damage << "-m" << " ";
                }else if(isI){
                    ss << damage << "-i" << " ";
                }else {
                    ss << damage << " ";
                }
            }
            if(counter == 10){
                ss << std::endl;
                counter = 0;
            }
        }
        auto turn = result3.turn;
        if (players[0].hp <= 0){
            ss << "L " << turn;
            analyzeData.setWinStatus(false);
        }
        if (players[1].hp <= 0){
            ss << "W " << turn;
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
              [](const AnalyzeData& a, const AnalyzeData& b) {
                  return a.calculateEfficiency() <= b.calculateEfficiency(); // 降順でソート
              });

    int lastTurn = -1;
    // ソートされた候補を処理
    int found = 0;
    for (AnalyzeData& data : candidate) {
        //std::cout << "efficiency: " << data.calculateEfficiency() << std::endl;
        if (data.calculateEfficiency() > 7){
            if (data.getWinStatus()){
                CandidateID++;
                found++;
                std::cout << std::endl << std::endl << "=======cid " << CandidateID << "========" << std::endl << data.getBattleTrace();
                std::cout << "actions: " << std::endl;

                auto vec = data.getGenome();
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] != 0) {
                        int action = (vec[i] & 0x3ff);
                        std::string actionName;
                        if (action == BattleEmulator::HEAL){
                            actionName = "HEAL";
                        }else if(action == BattleEmulator::DEFENCE){
                            actionName = "DEFENCE";
                        }
                        lastTurn = i+1;
                        std::cout << "turn: " << lastTurn << ", ehp: " << ((vec[i] >> 10) & 0x3ff) << ", ahp: " << ((vec[i] >> 20) & 0x3ff) << ", action: " << actionName <<  std::endl;
                    }
                }
            }
        }
        // 他の処理をここに追加...
    }

    if (found != 0){
        return;
    }

    // 最高の効率を持つ要素を見つける
    auto maxElementIter = std::max_element(candidate.begin(), candidate.end(),
                                           [](const AnalyzeData& a, const AnalyzeData& b) {
                                               return a.calculateEfficiency() < b.calculateEfficiency();
                                           });

    if (maxElementIter != candidate.end()) {
        // 最も高い効率値を持つ要素にアクセス
        AnalyzeData& bestCandidate = *maxElementIter;

    } else {
        std::cout << "error1" << std::endl;
    }
}

int toint(char * str){
    try {
        int number = std::stoi(str);
        return number;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return -1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return -1;
    }
}