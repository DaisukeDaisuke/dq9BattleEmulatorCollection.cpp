//
// Created by Owner on 2024/02/06.
//

#include <vector>
#include "camera.h"
#include "BattleEmulator.h"
#include "lcg.h"

void camera::Main(int *position, const int32_t actions[5], uint64_t * NowState, bool preemptive1) {
    bool preemptive = true;
    for (int i = 0; i < 3; ++i) {
        int32_t after = actions[i];
        if (after == BattleEmulator::ATTACK_ALLY||after == BattleEmulator::SKY_ATTACK||after == BattleEmulator::MERA_ZOMA) {
            onFreeCameraMove(position, after, preemptive ? 1 : 0, NowState);
        }
        preemptive = false;
    }
}

void camera::onFreeCameraMove(int *position, const int action, const int param5, uint64_t * NowState) {
    auto counter = ((*NowState) >> 16) & 0xff;
    do {
        if (param5 == 0) {
            (*position)++;
            if (counter == 0) {
                counter++;
                break;
            }
            auto ret = lcg::getPercent(position, 5 - counter);
            if (ret == 0 || counter == 5) {
                counter = 0;
                (*position) += 1;
            } else {
                counter++;
            }
        } else {
            (*position)++;
            if (counter == 0) {
                (*position)++;//引数5が1なら強制的に実行
                counter = 0;
                break;
            }
            (*position)++;
            counter = 0;
            (*position)++;

        }
    } while (false);
    (*NowState) &= ~0xff0000;
    (*NowState) |= (counter << 16);

}