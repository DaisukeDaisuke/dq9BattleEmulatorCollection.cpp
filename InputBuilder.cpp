#include "InputBuilder.h"

#include "BattleEmulator.h"

void InputBuilder::push(int damage, const char prefix) {
    InputEntry entry;
    entry.damage = damage;

    if (damage == -5) {
        entry.candidates.push_back(BattleEmulator::SPECIAL_MEDICINE);
    }else if (damage == -2) {
        entry.candidates.push_back(BattleEmulator::DECELERATLE);
    }else if (damage == -3) {
        entry.candidates.push_back(BattleEmulator::KASAP);
    }else if (damage == -4) {
        entry.candidates.push_back(BattleEmulator::SWEET_BREATH);
    }else if (damage == 0) {
        entry.candidates.push_back(BattleEmulator::ATTACK_ENEMY);
    }else

#ifdef BattleEmulatorLV19
    if (prefix == 'a') {
        if (damage > 30 && damage <= 60) {
            entry.candidates.push_back(BattleEmulator::MIRACLE_SLASH);
        } else {
            entry.candidates.push_back(BattleEmulator::ATTACK_ALLY);
        }
    } else {
        entry.candidates.push_back(BattleEmulator::ATTACK_ENEMY);
    }
#elifdef BattleEmulatorLV15
    if (prefix == 'a') {
        entry.candidates.push_back(BattleEmulator::ATTACK_ALLY);
    } else {
        entry.candidates.push_back(BattleEmulator::ATTACK_ENEMY);
    }
#endif

    if (entry.candidates.empty()) {
        std::cerr << "WARNING: A damage value of 0 " << damage << " has no applicable range\n";
    }
    inputs.push_back(entry);
}

std::vector<ResultStructure> InputBuilder::makeStructure() {
    int ambiguousCount = 0;
    for (const auto &entry: inputs) {
        if (entry.candidates.size() > 1)
            ++ambiguousCount;
    }
    if (ambiguousCount >= 4) {
        throw std::runtime_error("Error: Risk of combination explosion with three or more ambiguous inputs.\n");
    }

    std::vector<ResultStructure> results;
    ResultStructure current;
    generateCombinations(0, current, results);
    return results;
}

void InputBuilder::generateCombinations(size_t index, ResultStructure current, std::vector<ResultStructure> &results) {
    if (index == inputs.size()) {
        results.push_back(current);
        return;
    }

    // 入力の順番情報を保持するため、各入力のダメージを1度だけ追加
    current.AII_damage[current.AII_damageCounter++] = inputs[index].damage;

    const InputEntry &entry = inputs[index];
    for (int candidate: entry.candidates) {
        ResultStructure next = current; // 既にAII_damageが追加済み
        if (candidate == BattleEmulator::ATTACK_ENEMY || candidate == BattleEmulator::KASAP || candidate ==
            BattleEmulator::DECELERATLE || candidate == BattleEmulator::SWEET_BREATH) {
            next.Edamage[next.EdamageCounter++] = entry.damage;
        } else {
            next.Aactions[next.AactionsCounter++] = candidate;
            next.Adamage[next.AdamageCounter++] = entry.damage;
        }
        generateCombinations(index + 1, next, results);
    }
}
