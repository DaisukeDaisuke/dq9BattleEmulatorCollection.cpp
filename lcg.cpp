//
// Created by Owner on 2024/02/05.
//

#include "lcg.h"
#include <stdint.h>
#include <stdexcept>
#include <cmath>  // cmathヘッダーをインクルードする
#include <iostream>
#include <chrono>

// Define the size of the array
const int ARRAY_SIZE = 500;

double *precalculatedValues;
uint64_t *seeds;

lcg::lcg() {
    // コンストラクタの実装

}

lcg::~lcg() {
    delete[] precalculatedValues;
    // デストラクタの実装
    // メモリの解放やその他のクリーンアップ作業をここで行う
}

void lcg::init(uint64_t seed) {
    // Calculate and store values in the array
    precalculatedValues = new double[ARRAY_SIZE];
    seeds = new uint64_t[ARRAY_SIZE];
    for (int i = 1; i < ARRAY_SIZE; ++i) {
        seed = lcg_rand(seed);
        precalculatedValues[i] = calculatePercent(seed) * 0.01;
        seeds[i] = seed >> 32;
    }
}

void lcg::release() {
    delete[] precalculatedValues;
    delete[] seeds;
}

// Linear congruential generator function
uint64_t lcg::lcg_rand(uint64_t seed) {
    // Constants for the LCG formula
    const uint64_t multiplier = 0x5d588b656c078965;
    const uint64_t increment = 0x269ec3;
    const uint64_t modulo = 0xFFFFFFFFFFFFFFFF;

    // Update the seed using the LCG formula
    seed = seed * multiplier + increment;

    // Apply modulo to keep the result within the specified range
    seed = seed & modulo;

    return seed;
}

// Function to calculate percent in C++
double lcg::calculatePercent(uint64_t input) {
    // Right shift the input by 32 bits
    uint64_t output = input >> 32;

    // Multiply the result by 100
    output *= 100;

    // Divide the result by (2 << 31) and apply % 2
    return static_cast<double>(output) / (2ULL << 31);
}

int lcg::getPercent(int *position, int max) {
    // nullptrでないことを確認
    if (position == nullptr) {
        throw std::invalid_argument("Null pointer passed to incrementPosition.");
    }
    double result = precalculatedValues[*position];
    double scaledResult = result * max;

    // floorしてintにキャスト
    int roundedResult = static_cast<int>(floor(scaledResult));

    // ポインタの指す位置をインクリメント
    (*position)++;
    return roundedResult;
}

double lcg::floatRand(int * position, double min, double max){
    double result = precalculatedValues[*position];
    (*position)++;
    return min + result * (max - min);
}

int lcg::intRangeRand(int * position, int min, int max){
    return min + getPercent(position, max - min + 1);
}

uint64_t lcg::getSeed(int *position){
    if (position == nullptr) {
        throw std::invalid_argument("Null pointer passed to incrementPosition.");
    }
    uint64_t result = seeds[*position];
    (*position)++;
    return result;
}
