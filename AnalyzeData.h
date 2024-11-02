//
// Created by Owner on 2024/10/02.
//

#ifndef NEWDIRECTORY_ANALYZEDATA_H
#define NEWDIRECTORY_ANALYZEDATA_H

#include <utility>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <memory>
#include "BattleResult.h"

class AnalyzeData {
public:
    // ゲノムの初期化
    std::vector<int32_t> genome;

    AnalyzeData() : genome(100, 0) {}

    // ゲノム全体のセッター
    void setGenome(const std::vector<int32_t> &newGenome) {
        if (newGenome.size() != genome.size()) {
            throw std::invalid_argument("Genome size mismatch");
        }
        genome = newGenome;
    }

    void setWinStatus(const bool win) {
        winStatus = win;
    }

    [[nodiscard]] bool getWinStatus() const {
        return winStatus;
    }

    // ゲノム全体のゲッター
    [[nodiscard]] std::vector<int32_t> getGenome() const {
        return genome;
    }

    void setBattleTrace(std::string trace) {
        BattleTrace = std::move(trace);
    }

    [[nodiscard]] std::string getBattleTrace() const {
        return BattleTrace;
    }

    // BattleResult からデータを取得し、クラスのプロパティを設定
    void FromBattleResult(const BattleResult &result) {
        battleResult = std::make_shared<BattleResult>(result);
        turns = result.turn;
    }

    void setEvaluationString(std::string str) {
        EvaluationString = std::move(str);
    }

    [[nodiscard]] std::string getEvaluationString() {
        return EvaluationString;
    }

    void setLastInputTurn(int turn) {
        lastInputTurn = turn;
    }

    [[nodiscard]] int getLastInputTurn() {
        return lastInputTurn;
    }

    // バトル効率を計算
    [[nodiscard]] int calculateEfficiency() const {
        return turns;
    }

    // BattleResult を取得するゲッター
    [[nodiscard]] std::shared_ptr<BattleResult> getBattleResult() const {
        return battleResult;
    }

private:
    int turns = 0;
    bool winStatus = false;
    int lastInputTurn;
    std::shared_ptr<BattleResult> battleResult;
    int ehp = 0;
    int ahp = 0;
    std::string BattleTrace;
    std::string EvaluationString;
};


#endif //NEWDIRECTORY_ANALYZEDATA_H
