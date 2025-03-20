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

// 最大350ターン分の管理
constexpr int MAX_TURNS = 350;

class ActionBanManager {
private:
    // 各ターンごとに行動を記録するビットフィールド
    std::array<uint64_t, MAX_TURNS> ban_actions{};

public:
    // 指定ターンで行動をBANする
    void ban_action(int turn, int action) {
        if (turn < 0 || turn >= MAX_TURNS) {
            std::cerr << "Invalid turn value (must be between 0 and " << MAX_TURNS - 1 << ")\n";
            return;
        }
        if (action < 25 || action > 63) {
            std::cerr << "Invalid action value (must be between 25 and 51)" << action  << "\n";
            return;
        }
        ban_actions[turn] |= action_to_bit(action);
    }

    // 指定されたターン範囲内で行動がBANされているかをチェック
    bool is_action_banned(int action, int current_turn, int lookback_turns = 0) const {
        if (action < 25 || action > 63) {
            std::cerr << "Invalid action value (must be between 25 and 51): " << action  << "\n";
            return false;
        }
        if (current_turn < 0 || current_turn >= MAX_TURNS) {
            std::cerr << "Invalid current_turn value (must be between 0 and " << MAX_TURNS - 1 << ")\n";
            return false;
        }

        return (ban_actions[current_turn-lookback_turns] & action_to_bit(action)) != 0; // BANされている
    }

    // 指定ターンの行動をクリア（BAN解除）
    void clear_turn(int turn) {
        if (turn < 0 || turn >= MAX_TURNS) {
            std::cerr << "Invalid turn value (must be between 0 and " << MAX_TURNS - 1 << ")\n";
            return;
        }
        ban_actions[turn] = 0;
    }

    // デバッグ用：現在のBAN状態を表示
    void debug_print() const {
        for (int i = 0; i < MAX_TURNS; ++i) {
            std::cout << "Turn " << i << ": " << std::bitset<64>(ban_actions[i]) << "\n";
        }
    }
};

#endif //NEWDIRECTORY_ACTIONBANMANAGER_H
