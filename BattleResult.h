//
// Created by Owner on 2024/04/13.
//

#ifndef NEWDIRECTORY_BATTLERESULT_H
#define NEWDIRECTORY_BATTLERESULT_H

#include <cstring>

class BattleResult {

public:
    // 各メンバの内容を 0 にリセットする clear 関数
    void clear() {
        // std::memset(actions, 0, sizeof(actions));
        // std::memset(damages, 0, sizeof(damages));
        // std::memset(isEnemy, 0, sizeof(isEnemy));
        // std::memset(BuffTurnss, 0, sizeof(BuffTurnss));
        // std::memset(PoisonTurns, 0, sizeof(PoisonTurns));
        // std::memset(SpeedTurn, 0, sizeof(SpeedTurn));
        // std::memset(turns, 0, sizeof(turns));
        // std::memset(initiative, 0, sizeof(initiative));
        // std::memset(ehp, 0, sizeof(ehp));
        // std::memset(ahp, 0, sizeof(ahp));
        // std::memset(scTurn, 0, sizeof(scTurn));
        // std::memset(amp, 0, sizeof(amp));
        // std::memset(state, 0, sizeof(state));
        position = 0;
        turn = 0;
    }

    static void
    add(std::optional<BattleResult> &obj1, int action, int damage, bool isEnemy, int BuffTurns, int PoisonTurns, int speedTurn, int turn,
        bool player0_has_initiative, int ehp, int ahp, uint64_t nowState, int scTurn, int amp) {
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
};

#endif //NEWDIRECTORY_BATTLERESULT_H
