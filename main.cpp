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

int toint(char *string);

uint64_t tolong(char *string);

void processResult(BattleResult &result,const Player *copiedPlayers,const uint64_t seed);

std::vector<std::vector<int32_t>> genes;
std::vector<int32_t> original_genes;


using namespace std;


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "argc!!" << std::endl;
        return 1;
    }
    //https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/library-ios-iomanip#3.1-c-%E8%A8%80%E8%AA%9E%E3%81%AE%E5%85%A5%E5%87%BA%E5%8A%9B%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%A0%E3%81%A8%E3%81%AE%E5%90%8C%E6%9C%9F%E3%82%92%E7%84%A1%E5%8A%B9%E3%81%AB%E3%81%99%E3%82%8B
    std::cin.tie(0)->sync_with_stdio(0);

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<int32_t> gene = std::vector<int32_t>();

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

    int startHP = toint(argv[1]);
    uint64_t seed = tolong(argv[2]);
    players[0].hp = startHP;

    Player copiedPlayers[2];
    for (int i = 0; i < 2; ++i) {
        copiedPlayers[i] = players[i];
    }

//    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
//    totalSeconds = totalSeconds - 14;
//    //std::cout << totalSeconds << std::endl;
//    auto time1 = static_cast<uint64_t>(floor((totalSeconds - 1) * (1 / 0.12515)));
//    time1 = (time1 & 0xffff) << 16;
//    //std::cout << time1 << std::endl;
//
//    auto time2 = static_cast<uint64_t>(floor((totalSeconds + 1) * (1 / 0.125155)));
//    time2 = (time2 & 0xffff) << 16;
    //std::cout << time2  << std::endl;
    // Now you can use the precalculated values as needed
    //int test = 0;
    //std::vector<std::stringstream> streams(100);
    //for (uint64_t seed = 0; seed < 100000; ++seed) {
    //std::string str2 = "10 18 19 10 h 9 17 8 18 11 9 h 18";
    //std::string str2 = "19 9 9 17 h 13 15 10 10 11 11 h 15";
    //std::string str2 = argv[5];//"18 9 15 18 15 h 19 9 14";

//    time1 = 0x98087FD0;
//    time2 = 0x98087FD0+1;
    //for (uint64_t seed = time1; seed < time2; ++seed) {
    lcg::init(seed, 100000);
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
        auto player = result.players[i];
        auto isEnemy = result.isEnemy[i];
        if (action == BattleEmulator::HEAL || action == BattleEmulator::MEDICINAL_HERBS) {
            ss << "h ";
        } else if (damage != 0) {
            if (player.paralysis) {
                ss << damage << "m ";
            } else if (player.inactive) {
                ss << damage << "i ";
            } else {
                ss << damage << " ";
            }
        }
    }
    if (players[0].hp <= 0) {
        ss << "L " << result.turn[result.position - 1];
    }
    if (players[1].hp <= 0) {
        ss << "W " << result.turn[result.position - 1];
    }
    std::string resultStr = ss.str();
    std::cout << resultStr << std::endl;

    processResult(result, copiedPlayers, seed);

    lcg::release();
    delete position;



    //}
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

void processResult(BattleResult &result, const Player *copiedPlayers, const uint64_t seed) {
    std::stringstream ss1;
    int PreviousTurn = 0;
    int lastParalysis = -1;
    int lastCounter = -1;
    int paralysisTurns = 0;
    bool first = false;

    std::vector<int32_t> gene;

    for (int j = 0; j < result.position; ++j) {
        if (!result.isEnemy[j]) {
            gene.push_back(static_cast<int32_t>(result.actions[j]));
        }
    }

    for (int j = result.position - 1; j >= 0; --j) {
        int action = result.actions[j];
        if (!result.isEnemy[j]) {
            if (action == BattleEmulator::PARALYSIS || action == BattleEmulator::CURE_PARALYSIS ||
                action == BattleEmulator::INACTIVE_ALLY) {
                gene.pop_back();
            } else {
                break;
            }
        }
    }


    original_genes = gene;

//    auto lastPlayer = result.players[result.position-1];
//    if(lastPlayer.paralysis) {

    for (int j = result.position - 1; j >= 0; --j) {
        if (!result.isEnemy[j]) {
            // ss1 << lastParalysis << ": " << paralysisTurns << ", ";
            int action = result.actions[j];
            auto enemy = result.enemies[j];
            //ss1 << action << ": " << enemy.hp << ": " << result.turn[j] << ", ";
            if (action == BattleEmulator::PARALYSIS || action == BattleEmulator::CURE_PARALYSIS||(paralysisTurns != 0&&action == BattleEmulator::INACTIVE_ALLY)) {
                paralysisTurns++;
                lastParalysis = result.turn[j];
                lastCounter = j;
                continue;
            }
            if (paralysisTurns > 0) {
                first = true;
                //break;
            }
            if (first){
                break;
            }
        }
    }

    if (lastParalysis != -1) {
        int flag = 0;
        for (int j = lastCounter; j >= 0; --j) {
            int action = result.actions[j];
            int preemptive = result.preemptive[j];
            int turn = result.turn[j];
            auto enemy = result.enemies[j];
            if (action == BattleEmulator::UNSPEAKABLE_EVIL) {
                if(flag == 0) {
                    if (preemptive) {
                        flag = 1;
                    } else {
                        flag = 1;
                    }
                }

            }
            if (!result.isEnemy[j]) {
                if (action == BattleEmulator::PARALYSIS || action == BattleEmulator::CURE_PARALYSIS||(paralysisTurns != 0&&action == BattleEmulator::INACTIVE_ALLY)) {
                    continue;
                }
                if (flag == 2) {
                    if (turn > gene.size()){
                        break;
                    }
                    std::vector<int32_t> geneCopy;
                    geneCopy = gene;
                    ss1 << "mahi: hp: " << enemy.hp << ", turn: " << turn << std::endl;
                    geneCopy[turn] = BattleEmulator::DEFENCE;
                    genes.push_back(geneCopy);
                    flag++;
                    continue;
                }
                if(flag == 3){
                    if (turn > gene.size()){
                        break;
                    }
                    std::vector<int32_t> geneCopy;
                    geneCopy = gene;
                    ss1 << "mahi: hp: " << enemy.hp << ", turn: " << turn << std::endl;
                    geneCopy[turn] = BattleEmulator::DEFENCE;
                    genes.push_back(geneCopy);
                    flag++;
                    continue;
                }
                if(flag == 4){
                    if (turn > gene.size()){
                        break;
                    }
                    std::vector<int32_t> geneCopy;
                    geneCopy = gene;
                    ss1 << "mahi: hp: " << enemy.hp << ", turn: " << turn << std::endl;
                    geneCopy[turn] = BattleEmulator::DEFENCE;
                    genes.push_back(geneCopy);
                    flag++;
                    continue;
                }
                if(flag == 5){
                    if (turn > gene.size()){
                        break;
                    }
                    std::vector<int32_t> geneCopy;
                    geneCopy = gene;
                    ss1 << "mahi: hp: " << enemy.hp << ", turn: " << turn << std::endl;
                    geneCopy[turn] = BattleEmulator::DEFENCE;
                    genes.push_back(geneCopy);
                    flag++;
                    continue;
                }
                if(flag == 6){
                    if (turn > gene.size()){
                        break;
                    }
                    std::vector<int32_t> geneCopy;
                    geneCopy = gene;
                    ss1 << "mahi: hp: " << enemy.hp << ", turn: " << turn << std::endl;
                    geneCopy[turn] = BattleEmulator::DEFENCE;
                    genes.push_back(geneCopy);
                    flag++;
                    break;
                }
                if (flag != 0) {
                    flag++;
                }
            }
        }
    }

    for (const auto& gene : genes) {
        Player players[2];
        int *position = new int(1);
        for (int j = 0; j < 2; ++j) {
            players[j] = copiedPlayers[j];
        }
        BattleResult result1;

        BattleEmulator::Main(position, gene, players, result1, seed);
        if (players[0].hp <= 0) {
            cout << "L " << result1.turn[result1.position - 1] << std::endl;
        }
        if (players[1].hp <= 0) {
            cout << "W " << result1.turn[result1.position - 1] << std::endl;
        }
    }

    std::string resultStr1 = ss1.str();
    std::cout << resultStr1 << std::endl;
}

uint64_t tolong(char *str) {
    try {
        uint64_t number = std::stoull(str);  // stoullでunsigned long long型を取得
        return number;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return static_cast<uint64_t>(-1); // uint64_tとしてエラー時に-1を返す
    } catch (const std::out_of_range &e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return static_cast<uint64_t>(-1); // uint64_tとしてエラー時に-1を返す
    }
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

