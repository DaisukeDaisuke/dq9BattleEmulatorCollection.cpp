//
// Created by Owner on 2024/11/22.
//

#ifndef NEWDIRECTORY_GENOMELOGGER_H
#define NEWDIRECTORY_GENOMELOGGER_H

#include <array>
#include <iostream>
#include "Genome.h"

constexpr size_t LOG_SIZE = 10;

class GenomeLogger {
private:
    std::array<Genome, LOG_SIZE> logBuffer; // 固定サイズのリングバッファ
    size_t logIndex = 0;                    // 現在の書き込み位置
    size_t counter = 0;                     // 現在のターン数（処理回数）

public:
    // ログを保存する
    void saveLog(const Genome &genome) {
        logBuffer[logIndex] = genome;             // リングバッファに書き込み
        logIndex = (logIndex + 1) % LOG_SIZE;    // インデックスを循環
        ++counter;                               // 処理回数を増加
    }

    // N個前のログを取得する (1~9を指定)
    [[nodiscard]]     // N個前のログを取得する (1~9を指定)
    const Genome &getLog(size_t n) {
        if (n == 0 || n >= LOG_SIZE) {
            throw std::out_of_range("ログの範囲は1~9で指定してください");
        }

        // N個前のインデックスを計算
        size_t targetIndex = (logIndex + LOG_SIZE - n) % LOG_SIZE;

        logIndex = targetIndex;
        return logBuffer[targetIndex];
    }

    // 現在のターン数を取得
    [[nodiscard]] size_t getCounter() const {
        return counter;
    }
};

#endif //NEWDIRECTORY_GENOMELOGGER_H
