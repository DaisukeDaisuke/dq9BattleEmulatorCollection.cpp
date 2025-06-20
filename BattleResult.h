//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

/**
 * @class BattleResult
 * @brief バトルの結果を記録・管理するクラス
 *
 * このクラスは、各ターンにおける行動やダメージ、バフやデバフの状態など、バトルの詳細な結果を記録します。
 * また、記録されたデータを更新するためのメソッドを提供します。
 */
class BattleResult {

public:
    // 各メンバの内容を 0 にリセットする clear 関数
    void clear() {
        position = 0;
        turn = 0;
    }

    static void
    add(std::optional<BattleResult> &obj1, int action, int damage, bool isEnemy, int BuffTurns, int PoisonTurns, int speedTurn, int turn,
        bool player0_has_initiative, int ehp, int ahp, uint64_t nowState, int scTurn, int amp,bool defenseFlag) {
        if (obj1.has_value()) {
            BattleResult& obj = obj1.value();
            obj.actions[obj.position] = action;
            obj.damages[obj.position] = damage;
            obj.isEnemy[obj.position] = isEnemy;
            obj.BuffTurnss[obj.position] = BuffTurns;
            obj.PoisonTurns[obj.position] = PoisonTurns;
            obj.SpeedTurn[obj.position] = speedTurn;
            obj.turns[obj.position] = turn;
            obj.initiative[obj.position] = player0_has_initiative;
            obj.ehp[obj.position] = ehp;
            obj.ahp[obj.position] = ahp;
            obj.state[obj.position] = nowState;
            obj.scTurn[obj.position] = scTurn;
            obj.amp[obj.position] = amp;
            obj.defenseFlag[obj.position] = defenseFlag;
            obj.turn = turn;
            obj.position++;
        }
    }

    int position = 0;
    int turn = 0;
    int actions[1000] = {};
    int damages[1000] = {};
    int isEnemy[1000] = {};
    int BuffTurnss[1000] = {};
    int PoisonTurns[1000] = {};
    int SpeedTurn[1000] = {};
    int turns[1000] = {};
    bool initiative[1000] = {};
    int ehp[1000] = {};
    int ahp[1000] = {};
    int scTurn[1000] = {};
    int amp[1000] = {};
    uint64_t state[1000] = {};
    bool defenseFlag[1000] = {};
};

#endif //NEWDIRECTORY_BATTLERESULT_H
