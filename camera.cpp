//
// Created by Owner on 2024/02/06.
//

#include "camera.h"
#include "BattleEmulator.h"
#include "lcg.h"

int counter = 0;

void camera::Main(int * position, const uint32_t actions[5]){
    uint32_t before = -1;
    for (int i = 0; i < 5; ++i) {
        uint32_t after = actions[i];
        if (after == BattleEmulator::FIRE_BLOWING_ART){
            if (before == BattleEmulator::BOLT_CUTTER){
                (*position)++;
            }else{
                (*position)++;
            }

        }else if (
                (
                    before == BattleEmulator::FIRE_BLOWING_ART||
                    before == BattleEmulator::MERA||
                    before == BattleEmulator::BOLT_CUTTER
                )
            &&after == BattleEmulator::MERA
        ){
            onFreeCameraMove(position);
        }
        before = after;
    }
}

void camera::onFreeCameraMove(int * position){
    (*position)++;
    if (counter == 0){
        counter++;
        return;
    }
    auto ret = lcg::getPercent(position, 5 - counter);
    if(ret == 0||counter == 5){
        counter = 0;
        (*position) += 2;
    }
    counter++;
}