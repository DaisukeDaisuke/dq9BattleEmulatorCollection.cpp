//
// Created by Owner on 2024/02/05.
//

#include "lcg.h"
#include <cstdint>
#include <cmath>  // cmathヘッダーをインクルードする
#include <iostream>

// Define the size of the array
const int ARRAY_SIZE = 5000;

double precalculatedValues[ARRAY_SIZE]; // 固定メモリ
uint64_t seeds[ARRAY_SIZE];             // 固定メモリ
int nowCounter = 1;
uint64_t now_seed;

/**
 * 指定された乱数シードで線形合同法生成器を初期化します。
 *
 * @param seed 初期化に使用する乱数シード
 * @param init 必要に応じて初期乱数列をすべて生成するかどうかを制御するフラグ、trueに設定すると動的生成が無効になる。
 */
void lcg::init(uint64_t seed, bool init) {
    nowCounter = 1;
    now_seed = seed;

    if (init) {
        GenerateifNeed(ARRAY_SIZE - 3);
    }
}

/**
 * 必要に応じて乱数を追加生成します。
 * 指定されたインデックス位置までの値が計算されていない場合に、値を再計算して格納します。
 *
 * @param need 更新を必要とする配列のインデックス
 */
void lcg::GenerateifNeed(int need) {
    // 配列に値を再計算して格納する
    if(nowCounter > need){
        return;
    }
    for (int i = nowCounter; i < need+2; ++i) {
        now_seed = lcg_rand(now_seed);
        precalculatedValues[nowCounter] = calculatePercent(now_seed) * 0.01;
        seeds[nowCounter++] = now_seed >> 32;
    }
}

/**
 * 線形合同法（LCG）を使用して次の擬似乱数を生成します。
 *
 * @param seed 擬似乱数生成のための現在のシード値
 * @return 計算された次の擬似乱数シード値
 */
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

/**
 * 入力された値から線形合同法に基づいて百分率を計算します。
 * これはdq9と一定の互換性のある処理になっています。
 *
 * @param input 計算の基となる64ビットの入力値
 * @return 計算された百分率を表す値
 */
double lcg::calculatePercent(uint64_t input) {
    // Right shift the input by 32 bits
    uint64_t output = input >> 32;

    // Multiply the result by 100
    output *= 100;

    // Divide the result by (2 << 31) and apply % 2
    return static_cast<double>(output) / (2ULL << 31);
}

/**
 * 指定された位置と最大値を使用して、線形合同法による乱数を整数として取得します。
 * 位置は計算後に自動的にインクリメントされます。
 *
 * @param position 乱数の現在の位置を保持するポインタ
 *                 nullptrの場合は例外がスローされます。
 * @param max 結果の最大値。0~[最大-1]までを返す
 * @return 0以上max-1未満の整数値
 *         ただし、範囲外エラーが発生した場合は0を返します。
 * @throw std::invalid_argument positionがnullptrの場合
 */
int lcg::getPercent(int *position, int max) {
    // nullptrでないことを確認
    if (position == nullptr) {
        throw std::invalid_argument("Null pointer passed to incrementPosition.");
    }
    if ((*position) >= ARRAY_SIZE) {
        std::cerr << "out of range!!!" << std::endl;
        return 0;
    }
    GenerateifNeed((*position));
    double result = precalculatedValues[*position];
    double scaledResult = result * max;

    // floorしてintにキャスト
    int roundedResult = static_cast<int>(floor(scaledResult));

    // ポインタの指す位置をインクリメント
    (*position)++;
    return roundedResult;
}

/**
 * 線形合同法生成器を使用して指定された範囲内でランダムな浮動小数点値を生成します。
 *
 * @param position 乱数生成の進行位置を示すポインタ。呼び出しごとに位置が更新されます。
 * @param min 生成される乱数の最小値（範囲の下限）。
 * @param max 生成される乱数の最大値（範囲の上限）。
 * @return
 *   指定された範囲 [min, max] 内のランダムな浮動小数点値。
 *   ただし最大値と最小値に一致する乱数は全部に返却されないことに注意する必要があります。
 * @throws std::invalid_argument 引数 position が nullptr の場合にスローされます。
 */
double lcg::floatRand(int *position, double min, double max) {
    if (position == nullptr) {
        throw std::invalid_argument("Null pointer passed to incrementPosition.");
    }
    if ((*position) >= ARRAY_SIZE) {
        std::cerr << "out of range!!!" << std::endl;
        return 0;
    }
    GenerateifNeed((*position));
    double result = precalculatedValues[*position];
    (*position)++;
    return min + result * (max - min);
}

/**
 * 指定された範囲内で整数型の乱数を生成します。
 *
 * @param position 現在の生成位置を表すポインタ
 * @param min 生成する乱数の最小値
 * @param max 生成する乱数の最大値
 * @return minからmaxの範囲内の乱数
 *
 * @throw std::invalid_argument positionがnullptrの場合にスローされます。
 * @note 指定されたpositionはインクリメントされます。
 * @note minおよびmaxは端の値を含みます。
 */
int lcg::intRangeRand(int *position, int min, int max) {
    return min + getPercent(position, max - min + 1);
}

uint64_t lcg::getSeed(int *position) {
    if (position == nullptr) {
        throw std::invalid_argument("Null pointer passed to incrementPosition.");
    }
    GenerateifNeed((*position));
    uint64_t result = seeds[*position];
    (*position)++;
    return result;
}