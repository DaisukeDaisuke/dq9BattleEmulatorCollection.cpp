//
// Created by Owner on 2024/11/22.
//

#ifndef NEWDIRECTORY_ACTIONBANMANAGER_H
#define NEWDIRECTORY_ACTIONBANMANAGER_H

#include <cstdint>
#include <bitset>
#include <array>
#include <iostream>

// constexpr関数で固有のビットインデックスを生成（-24して管理）
constexpr uint64_t action_to_bit(int action) {
    return static_cast<uint64_t>(1) << (action - 24);
}

// 最大10ターン分の管理
constexpr int MAX_TURNS = 10;

class ActionBanManager {
private:
    // 各ターンごとに行動を記録するビットフィールド
    std::array<uint64_t, MAX_TURNS> ban_actions{};
    int current_turn = 0;

public:
    // 現在のターンのインデックスを取得
    int get_current_turn() const {
        return current_turn;
    }

    // 現在のターンで行動をBANする
    void ban_current_turn_action(int action) {
        if (action < 25 || action > 51) {
            std::cerr << "Invalid action value (must be between 25 and 51)\n";
            return;
        }
        ban_actions[current_turn] |= action_to_bit(action);
    }

    // 過去1ターンで指定された行動がBANされているかをチェック
    bool is_action_banned(int action) const {
        if (action < 25 || action > 51) {
            std::cerr << "Invalid action value (must be between 25 and 51)\n";
            return false;
        }
        return (ban_actions[current_turn] & action_to_bit(action)) != 0;
    }


    // 次のターンに進む（古いデータを破棄）
    void advance_turn() {
        current_turn = (current_turn + 1) % MAX_TURNS;
        ban_actions[current_turn] = 0; // 古いデータをクリア
    }

    // 過去1ターンのインデックスを計算
    void previous_turn() {
        current_turn = ((current_turn - 1 + MAX_TURNS) % MAX_TURNS);
    }


    // デバッグ用：現在のBAN状態を表示
    void debug_print() const {
        for (int i = 0; i < MAX_TURNS; ++i) {
            std::cout << "Turn " << i << ": " << std::bitset<64>(ban_actions[i]) << "\n";
        }
    }
};


#endif //NEWDIRECTORY_ACTIONBANMANAGER_H
