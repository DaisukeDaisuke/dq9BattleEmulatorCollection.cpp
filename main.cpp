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
#include "lcg.h"
#include "BattleEmulator.h"

using namespace std;


int main() {
    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    std::cin.tie(0)->sync_with_stdio(0);

    auto t0 = std::chrono::high_resolution_clock::now();

    int32_t gene[100] = {
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::HEAL,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::HEAL,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::ATTACK_ALLY,
            BattleEmulator::ATTACK_ALLY,

    };

    Player players[2];
    int hps[2] = {79, 456};
    int defs[2] = {73, 58};
    int atks[2] = {67, 56};
    int speeds[2] = {51, 54};
    int mps[2] = {27, 255};
    for (int i = 0; i < 2; ++i) {
        players[i].playerId = i;
        players[i].hp = hps[i];
        players[i].maxHp = hps[i];
        players[i].atk = atks[i];
        players[i].def = defs[i];
        players[i].speed = speeds[i];
        players[i].mp = mps[i];
    }
    players[0].hp = 70;

    Player copiedPlayers[2];
    for (int i = 0; i < 2; ++i) {
        copiedPlayers[i] = players[i];
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

    int hours = 1;
    int minutes = 21;
    int seconds = 28;
//    int hours = 1;
//    int minutes = 27;
//    int seconds = 58;
    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    totalSeconds = totalSeconds - 17;
    std::cout << totalSeconds << std::endl;
    auto time1 = static_cast<uint64_t>(floor((totalSeconds - 1) * (1 / 0.12515)));
    time1 = (time1 & 0xffff) << 16;
    std::cout << time1 << std::endl;

    auto time2 = static_cast<uint64_t>(floor((totalSeconds + 1) * (1 / 0.125155)));
    time2 = (time2 & 0xffff) << 16;
    std::cout << time2  << std::endl;
    // Now you can use the precalculated values as needed
    //int test = 0;
    //std::vector<std::stringstream> streams(100);
    //for (uint64_t seed = 0; seed < 100000; ++seed) {
    //std::string str2 = "10 18 19 10 h 9 17 8 18 11 9 h 18";
    std::string str2 = "19 9 9 17 h 13 15 10 10 11 11 h 15";

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
        BattleEmulator::Main(position, gene, players, result, seed);
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
        std::string resultStr = ss.str();

        if(resultStr.find(str2)!=std::string::npos){
            //std::cout << seed << std::endl;
            std::cout << resultStr << std::endl;
        }

//        if (resultStr.size() <= 149) {
//            int paddingSize = 149 - resultStr.size();
//            std::string padding(paddingSize, '-');
//            resultStr += padding;
//        }
//        if (result.position >= 2) {
//            int damage1;
//            for (int i = 0; i < result.position; ++i) {
//                auto action1 = result.actions[i];
//                damage1 = result.damages[i];
//                if (damage1 == 0) {
//                    continue;
//                }
//                if (damage1 == 1) {
//                    std::cout << "a" << std::endl;
//                }
//                if (action1 == BattleEmulator::HEAL || action1 == BattleEmulator::MEDICINAL_HERBS) {
//                    damage1 = 0;
//                }
//                break;
//            }


//            auto action2 = result.actions[1];
//            auto damage2 = result.damages[1];
//            if (action2 == BattleEmulator::HEAL || action2 == BattleEmulator::MEDICINAL_HERBS) {
//                damage2 = 0;
//            }
//            streams.at(damage1) << resultStr << std::endl;
//        }
        //std::cout << resultStr << std::endl;
        lcg::release();
        delete position;
    }

    // ファイルに記録する
//    for (int i = 0; i < 100; ++i) {
//        //for (int j = 0; j < 100; ++j) {
//        if (!streams[i].str().empty()) {
//            std::string filename = "output/output_" + std::to_string(i) + ".txt";
//            std::ofstream outfile(filename);
//            if (outfile.is_open()) {
//                // 空のストリームは無視
//                outfile << streams[i].str() << std::endl;
//                outfile.close();
//            } else {
//                std::cerr << "file error " << filename << std::endl;
//                return 1;
//            }
//        }
//    }

//    for (int i = 0; i < 100; ++i) {
//        //test += lcg::getPercent(position, 100);
//        //std::cout << lcg::floatRand(position, 1.5, 2.5) << std::endl;
//    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms"
              << std::endl;
    //std::cout << (*position) << std::endl;

    //cout << "hp" << endl;
    for (auto &player: players) {
        std::cout << player.hp << std::endl;
    }
    //std::cout << (players[0].specialCharge ? "hissatu: true" : "hissatu: fa") << std::endl;

    //std::cout << "Size of int: " << sizeof(int) * 8 << " bits" << std::endl;

    //delete position;


    return 0;
}
